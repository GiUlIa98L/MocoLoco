#include "./TwoBit/twobit.c"
#include "./TwoBit/twobit.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_sf.h>
#include <gsl/gsl_sf_bessel.h>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>
#include <iomanip>

using namespace std;

#pragma once
#include <algorithm>
#include <thread>

#define PROFILING 1
#if PROFILING
#define PROFILE_SCOPE(name) InstrumentationTimer timer##__LINE__(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__PRETTY_FUNCTION__)
#else
#define PROFILE_SCOPE(name)
#endif

string BED_FILE;
int half_length = 150;
string TWOBIT_FILE;
string JASPAR_FILE;
string alias_file = "multifasta_";
string MFASTA_FILE;
string ordering;
const unsigned int overhead = 25;
const double pseudoc = 0.01;
bool DS = true;
string kmers = "6,8,10";
string dist = "1,2,3";
unsigned int top_N = 10;
double freq_treshold = 0.02;
bool local_maxima_grouping = true;
bool refining_matrix = false;
string exp_max;
bool tomtom = false;
bool err = false;
vector<bool> rev;
string reverse_sequence;
string reverse_matrix_seq;
string oligos_reverse;
TwoBit *tb;
double sim_tresh = 0.001;
bool direction = false;

class Timer {
public:
  Timer() { m_StartTimepoint = chrono::high_resolution_clock::now(); }
  ~Timer() { Stop(); }

  void Stop() {
    auto endTimepoint = chrono::high_resolution_clock::now();

    auto start = chrono::time_point_cast<chrono::microseconds>(m_StartTimepoint)
                     .time_since_epoch()
                     .count();
    auto end = chrono::time_point_cast<chrono::microseconds>(endTimepoint)
                   .time_since_epoch()
                   .count();

    auto duration = end - start;
    double ms = duration * 0.000001;

    cout << duration << "us (" << ms << "s)\n";
  }

private:
  chrono::time_point<chrono::high_resolution_clock> m_StartTimepoint;
};

class bed_class {

private:
  string chr_coord;
  unsigned int start_coord;
  unsigned int end_coord;
  string sequence;
  bool flag;

  friend class coordinator_class;
  friend class z_test_class;
  friend class map_class;

  void read_line(string);

public:
  // Bed class constructor if input is a Multifasta file
  bed_class(string seq) {

    chr_coord = "MULTIFASTA";
    start_coord = 0;
    end_coord = 0;
    sequence = seq;
  }

  // Bed class constructor if input are Bed-Twobit-Jaspar files
  bed_class(string line, TwoBit *tb, unsigned int n_line) {

    // Take line from bed file and extract chr_coord, start_coord and end_coord
    read_line(line);

    // Set the new start and end coordinates following p (half_length) input and
    // add overhead to end coordinates
    centering_function(start_coord, end_coord, half_length, overhead);

    // Extract from twobit genome the sequence following chr, start and end
    // coordinates
    extract_seq(tb, n_line);
  }

  void centering_function(unsigned int, unsigned int, int, const unsigned int);
  void extract_seq(TwoBit *, unsigned int);
};

class matrix_class {

private:
  vector<vector<double>> matrix;
  vector<vector<double>> norm_matrix;
  vector<vector<double>> inverse_norm_matrix;
  vector<vector<double>> matrix_log;
  vector<vector<double>> inverse_matrix_log;
  vector<double> col_sum;

  void matrix_normalization_pseudoc(vector<vector<double>> &);
  void matrix_normalization(vector<vector<double>> &);
  void matrix_logarithmic(vector<vector<double>> &);
  vector<vector<double>> reverse_matrix(vector<vector<double>> &);
  vector<double> find_col_sum(vector<vector<double>> &);
  void print_debug_matrix(vector<vector<double>>, string);

public:
  matrix_class(vector<vector<double>> &mat) {

    // Function to normalize the matrix scores and add a pseudocount
    matrix_normalization_pseudoc(mat);

    // Function to normalize again the matrix after pseudocount addition
    matrix_normalization(norm_matrix);

    // Function to calculate logarithmic matrix from normalized matrix
    matrix_logarithmic(norm_matrix);

    // Function to reverse the logarithmic normalized matrix to read the oligo
    // in reverse strand
    inverse_matrix_log = reverse_matrix(matrix_log);
  }

  void debug_matrix(matrix_class);
  vector<vector<double>> return_log_matrix();
  vector<vector<double>> return_inverse_log_matrix();
  vector<vector<double>> return_norm_matrix();
  vector<vector<double>> return_matrix();
};

class oligo_class {

private:
  vector<double> oligo_scores;
  vector<double> o_matrix_mins;
  vector<double> o_matrix_maxes;
  double min_possible_score;
  double max_possible_score;
  double best_score;
  string global_sequence;
  string best_oligo_seq;
  unsigned int local_position;
  string chr_coord_oligo;
  unsigned int start_coord_oligo;
  unsigned int end_coord_oligo;
  char strand;

  void find_minmax(vector<vector<double>> &);
  unsigned int find_best_score();
  void find_coordinate(unsigned int, string, unsigned int);
  void find_best_sequence(string, unsigned int);
  void scores_normalization();
  friend class coordinator_class;
  friend class z_test_class;

public:
  oligo_class(vector<vector<double>> &matrix, string &sequence,
              string chr_coord_GEP, unsigned int start_coord_GEP,
              char strand_sign) {

    // Function to annotate in min_possible_score and max_possible_score the
    // worst and the best score that an oligo can reach based on the current
    // jaspar matrix
    find_minmax(matrix);

    // for each oligo in the current sequence a total score of similarity is
    // calculated against the JASPAR matrix
    shifting(matrix, sequence);

    // Function to normalize the scores with the normalization formula
    scores_normalization();

    // Find the best score position and save it into local_position variable (If
    // find more than one select as best the nearest to the center
    local_position = find_best_score();

    // Function to extract and save the best oligo sequence
    find_best_sequence(sequence, matrix[0].size());

    // The coordinates of the best oligo are saved --> It will be useful to
    // center the window on the best oligo find_coordinate(matrix[0].size(),
    // chr_coord_GEP, start_coord_GEP);

    strand = strand_sign;

    global_sequence = sequence;

    chr_coord_oligo = chr_coord_GEP;

    start_coord_oligo = start_coord_GEP + local_position;

    end_coord_oligo = start_coord_oligo + matrix[0].size();
  }

  oligo_class(vector<vector<double>> &matrix, string &sequence) {

    find_minmax(matrix);

    shifting(matrix, sequence);

    scores_normalization();
  }

  void shifting(vector<vector<double>> &, string &);
};

class coordinator_class { // Coordinator class to connect Matrix to Bed and
                          // Oligos_vector

private:
  vector<vector<double>> matrix;
  vector<vector<double>> matrix_log;
  vector<vector<double>> inverse_matrix_log;
  vector<oligo_class> oligos_vector;

  void centering_oligo();
  void best_strand();
  void GEP_creation(vector<bed_class> &);
  void oligos_vector_creation(vector<oligo_class> &, vector<vector<double>> &,
                              vector<vector<double>> &, vector<bed_class> &);
  vector<vector<double>> read_JASPAR();

public:
  coordinator_class() {

    // GEP (vector of bed class) creation. An empty GEP vector is passed by
    // reference to be filled and saved in this class
    GEP_creation(GEP);

    // reading Jaspar file and returning scores as a matrix of double, saved in
    // a variable called matrix
    read_JASPAR();

    // Creating matrix class: input matrix scores, name and tf matrix name
    matrix_class M(matrix);

    // matrix_log and inverse_matrix_log calculated are returned from Matrix
    // class to be saved here --> These are the two matrices on which the
    // analysis will be performed
    matrix_log = M.return_log_matrix();
    inverse_matrix_log = M.return_inverse_log_matrix();

    // The sequences are shifting on the matrix and, for each position, an oligo
    // score is calculated, based on log and inverse_log matrices An oligo_class
    // is created for each sequence shifting to analyze all oligo scores. All
    // the information are saved on oligos_vector, which is an oligo_class
    // vector passed by reference to this function and filled sequence by
    // sequence.
    oligos_vector_creation(oligos_vector, matrix_log, inverse_matrix_log, GEP);

    // If the analysis is performed on Double Strand the function best_strand is
    // useful to select if an oligo has the best score on FWD or REV strand,
    // discarding the other
    if (DS) {
      best_strand();
    }

    // The best oligo selected for each sequence becames the new center of the
    // window, re-setting the GEP coordinates
    centering_oligo();
  }

  vector<bed_class> GEP;

  void print_GEP(vector<bed_class> &);
};

class multifasta_class {

private:
  vector<string> sequences;

  void length_control(vector<string>);
  void extract_sequences();
  void GEP_creation_MF(vector<string>);
  void alias_output_filename();

public:
  vector<bed_class> GEP;

  multifasta_class(string MFASTA_FILE) {

    // Firstly the fasta sequences from multifasta file are extracted and saved
    // into a vector of strings
    extract_sequences();

    // Then the length control is performed --> All the MF sequences must be of
    // the same langth
    length_control(sequences);

    // Function to handle output names
    alias_output_filename();

    // For every sequence in vector "sequences" a bed class is created to store
    // the FASTA seqinto a GEP vector
    GEP_creation_MF(sequences);
  }
};

class p_value_class {

private:
  vector<unsigned int> K_vec;
  vector<unsigned int> N1_vec;
  vector<unsigned int> N2_vec;
  unsigned int T;
  vector<double> p_value_vec;
  string reverse_bases;
  string oligo;
  string oligo_RC;
  multimap<pair<unsigned int, unsigned int>, pair<string, string>>
      vertical_multimap;
  unordered_map<string, unsigned int>::iterator it_N1_plus;
  unordered_map<string, unsigned int>::iterator it_N1_minus;
  unsigned int total_oligo_N2;
  unsigned int position;
  unsigned int rank;
  unsigned int sum_top_N;
  double p_val;
  double p_val_log10;
  multimap<double, pair<string, string>> p_value_sort;
  multimap<double, pair<string, vector<unsigned int>>> p_value_KNT;

  multimap<unsigned int, pair<string, string>>
  creating_sum_occurrences_multimap(
      multimap<pair<unsigned int, unsigned int>, pair<string, string>> &);
  void multimap_creation(
      map<pair<string, string>, pair<unsigned int, unsigned int>>);
  void filling_KNT_vectors(unordered_map<string, unsigned int> &);
  void N2_calculation(unordered_map<string, unsigned int> &);
  void calculating_p_value();
  void sorting_p_value();
  void checking_ordering(
      map<pair<string, string>, pair<unsigned int, unsigned int>> &,
      unsigned int, ofstream &, unsigned int);
  void print_debug_p_value_DS(
      map<pair<string, string>, pair<unsigned int, unsigned int>> &,
      unsigned int, ofstream &, unsigned int);
  void print_debug_p_value_SS(
      map<pair<string, string>, pair<unsigned int, unsigned int>> &,
      unsigned int, ofstream &, unsigned int);
  void print_debug_occurrences_DS(
      map<pair<string, string>, pair<unsigned int, unsigned int>> &,
      unsigned int, ofstream &, unsigned int, vector<double>);
  void print_debug_occurrences_SS(
      map<pair<string, string>, pair<unsigned int, unsigned int>> &,
      unsigned int, ofstream &, unsigned int, vector<double>);
  friend class map_class;

public:
  p_value_class(
      map<pair<string, string>, pair<unsigned int, unsigned int>> &pair_map,
      unordered_map<string, unsigned int> &orizzontal_map, unsigned int t,
      unsigned int position, ofstream &outfile, unsigned int freq) {

    T = t;

    // From the vertical positional map (composed by two pair) a vertical
    // multimap is generated to sorting oligos by their occurrences
    multimap_creation(pair_map);

    // N2 parameter found doing the sum of the occurrences of all the oligos
    // along the sequences (sum of the occurrences in the horizontal map) -> The
    // real N2 value is calculated subsequently subtracting N1 value
    N2_calculation(orizzontal_map);

    // Function to calculate, for each oligo, its K/N1/N2 parameters. This
    // function allows to store these values into vectors, useful to print them
    // subsequentl
    filling_KNT_vectors(orizzontal_map);

    // Once K,N1,N2,T are calculated for each oligo they can be used to define
    // an oligo-specific p_value using a function from gsl library called
    // "gsl_cdf_hypergeometric_Q"
    calculating_p_value();

    // Sorting oligos in position by lowest p_value
    sorting_p_value();

    // Check the input parameters to select the rigth output printing
    // 1. Output file with oligos ordered by occurrences
    // 2. Output file with oligos ordered by lowest p_value
    checking_ordering(pair_map, position, outfile, freq);
  }
};

class hamming_class {

private:
  vector<string> best_oligos;
  string real_best_oligo;
  unsigned int real_best_oligo_occurrences;
  vector<string> similar_oligos;
  unsigned int number_first_hamming;
  vector<unsigned int> similar_oligos_occurrences;
  double tot_similar_occurrences;
  unsigned int kmer;
  double FREQUENCE_1;
  double FREQUENCE_2;
  vector<vector<double>> PWM_hamming;
  vector<vector<double>> norm_matrix;
  map<string, double> like_ratio_map;
  pair<double, double> FREQUENCE;
  unsigned int position;
  double comparison;
  map<string, double> similar_oligos_map;
  string reverse_bases;
  bool pal;
  multimap<unsigned int, pair<string, string>> sum_occurrences_multimap;

  friend class map_class;
  friend class p_value_class;

  multimap<unsigned int, pair<string, string>>
  creating_sum_occurrences_multimap(
      multimap<pair<unsigned int, unsigned int>, pair<string, string>> &);
  void find_best_oligos(
      multimap<pair<unsigned int, unsigned int>, pair<string, string>> &,
      multimap<unsigned int, pair<string, string>> &);
  void checking_best_oligo(
      unsigned int,
      multimap<pair<unsigned int, unsigned int>, pair<string, string>> &);
  void find_secondary_hamming(
      unsigned int, unsigned int,
      multimap<pair<unsigned int, unsigned int>, pair<string, string>> &);
  void find_distanced_oligos(
      string, unsigned int,
      multimap<pair<unsigned int, unsigned int>, pair<string, string>> &);
  string select_real_best_oligo(
      unsigned int,
      multimap<pair<unsigned int, unsigned int>, pair<string, string>> &);
  bool is_similar_oligo(string, string, unsigned int);
  bool checking_neighbour_presence(string);
  void print_debug_hamming(unsigned int, ofstream &);
  double frequence_1_calculation(unsigned int);
  double frequence_2_calculation(unordered_map<string, unsigned int> &,
                                 unordered_map<string, unsigned int> &,
                                 unsigned int);
  unsigned int
  finding_orizzontal_occurrences(unordered_map<string, unsigned int> &,
                                 unordered_map<string, unsigned int> &);
  void PWM_hamming_creation();
  void EM_Ipwm(vector<vector<double>> &);
  void EM_Epart(unordered_map<string, unsigned int> &,
                unsigned int, unordered_map<string, unsigned int> &);
  void EM_Mpart(unsigned int, unordered_map<string, unsigned int> &);
  bool EM_convergence(vector<vector<double>> &, vector<vector<double>> &, bool);
  void EM_cycle(unordered_map<string, unsigned int> &,
                unsigned int, unordered_map<string, unsigned int> &);
  void print_PWM(string, vector<vector<double>>);

public:
  // Occurrences constructor
  hamming_class(
      multimap<pair<unsigned int, unsigned int>, pair<string, string>>
          &vertical_multimap,
      map<pair<string, string>, pair<unsigned int, unsigned int>> &vertical_map,
      multimap<double, pair<string, string>> &p_value_sort,
      unsigned int distance, unsigned int position, unsigned int freq,
      unordered_map<string, unsigned int> &orizzontal_map_plus,
      unordered_map<string, unsigned int> &orizzontal_map_minus,
      ofstream &outfile,
      vector<unsigned int> kmers_vector) {

    sum_occurrences_multimap =
        creating_sum_occurrences_multimap(vertical_multimap);

    if (ordering == "p") {

      multimap<double, pair<string, string>>::iterator it_p_val =
          p_value_sort.begin();
      multimap<pair<string, string>, pair<unsigned int, unsigned int>>::iterator
          it_occ = vertical_map.begin();

      real_best_oligo = it_p_val->second.first;
      best_oligos.emplace_back(real_best_oligo);
      it_occ = vertical_map.find(
          make_pair(real_best_oligo, it_p_val->second.second));
      real_best_oligo_occurrences =
          it_occ->second.first + it_occ->second.second;

      find_distanced_oligos(real_best_oligo, distance, vertical_multimap);
    }

    else {
      // Saving the vertical multimap passed to constructor locally
      // kmer = kmers_vector[0];
      // Find the best oligo (by occurrences) scrolling the vertical multimap
      find_best_oligos(vertical_multimap, sum_occurrences_multimap);

      // Checking if best oligo is one or more. If more, do the selection to
      // find the real_best_oligo, else proceed to find hamming neighbours
      checking_best_oligo(distance, vertical_multimap);
    }
    // Save the oligo size --> to avoid infinite cycle for its continous
    // updating in the next function
    number_first_hamming = similar_oligos.size();

    if (refining_matrix) {
      // Finding hamming of all similar oligos
      find_secondary_hamming(distance, number_first_hamming, vertical_multimap);
    }

    // Adding the real best oligo to similar oligos vector (created starting
    // from itself)
    similar_oligos.emplace_back(real_best_oligo);

    // Adding real best oligo occurrences to similar occurrences vector
    similar_oligos_occurrences.emplace_back(real_best_oligo_occurrences);

    // Calculating the frequence 1 (total of similar occurrences / total of
    // possible oligos in the position) and saving it to FREQUENCE_1 variable
    frequence_1_calculation(freq);

    // Function to calculate the frequence_2 (total of similar occurrences /
    // total number of best+hamming occurrences in sequences)
    frequence_2_calculation(orizzontal_map_plus, orizzontal_map_minus,
                            position);

    // Print positional hamming features in hamming output file
    print_debug_hamming(position, outfile);

    // Building a PWM matrix from best oligo sequence and his hamming neigbours
    // sequences and occurrences
    PWM_hamming_creation();

    if (exp_max.length() > 0) {
      EM_Ipwm(PWM_hamming);
      EM_cycle(orizzontal_map_minus, position, orizzontal_map_plus);
      if (tomtom) {
        for (unsigned int x = 0; x < PWM_hamming.size(); x++) {
          for (unsigned int y = 0; y < PWM_hamming[0].size(); y++) {
            PWM_hamming[x][y] = round(PWM_hamming[x][y] * 100);
          }
        }
      }
    }
  }
};

class z_test_class {

private:
  // zscores and PWM pvalues

  vector<vector<double>> matrix_log;
  vector<vector<double>> inverse_matrix_log;
  vector<double> oligo_scores_orizzontal_FWD;
  vector<double> oligo_scores_orizzontal_REV;
  vector<double> oligo_scores_orizzontal_BEST;
  vector<double> all_global_scores;
  vector<double> all_local_scores;
  friend class map_class;

  void print_debug_oligo_vec(vector<vector<double>> &);
  void oligos_vector_creation_PWM(vector<bed_class> &);
  void z_score_parameters_calculation();
  void check_best_strand_oligo();
  void z_score_calculation();

public:
  double z_score;
  double Zpvalue;

  double global_mean;
  double global_dev_std;
  double local_mean;
  double local_dev_std;
  unsigned int local_pos;
  z_test_class(vector<vector<double>> &PWM_hamming, vector<bed_class> &GEP,
               unsigned int local_p, vector<unsigned int> kmers_vector) {

    local_pos = local_p;

    // Calling matrix class constructor passing PWM_hamming matrix
    matrix_class PWM_hamming_mat(PWM_hamming);

    // Return from matrix class the log_PWM_hamming matrix
    matrix_log = PWM_hamming_mat.return_log_matrix();

    if (DS) {

      // if analysis is in DS return from matrix class the
      // inverse_log_PWM_hamming matrix to shift the reverse strand
      inverse_matrix_log = PWM_hamming_mat.return_inverse_log_matrix();
    }

    // Function to shift PWM_matrix on sequences using oligo class functions
    oligos_vector_creation_PWM(GEP);

    // From local and global scores calculated finding all the parameters useful
    // to z-score calculation
    z_score_parameters_calculation();

    // Calculating z-score and p-value from it
    z_score_calculation();

    all_local_scores.clear();
    all_global_scores.clear();
    oligo_scores_orizzontal_FWD.clear();
    oligo_scores_orizzontal_REV.clear();
  }
};

class map_class {

private:
  vector<vector<map<pair<string, string>, pair<unsigned int, unsigned int>>>>
      vector_kmers_maps_plus;
  vector<unordered_map<string, unsigned int>> orizzontal_plus_debug;
  vector<unordered_map<string, unsigned int>> orizzontal_minus_debug;
  vector<map<pair<string, string>, pair<unsigned int, unsigned int>>>
      maps_vector_positions_plus;
  unordered_map<string, unsigned int> orizzontal_plus;
  unordered_map<string, unsigned int> orizzontal_minus;
  map<pair<string, string>, pair<unsigned int, unsigned int>> vertical_plus;
  string reverse_bases;
  vector<vector<unsigned int>> tot_freq_matrix;
  vector<unsigned int> tot_sum_vector;
  vector<vector<unsigned int>> tot_sum_matrix;
  vector<unsigned int> kmers_vector;
  vector<unsigned int> distance_vector;
  vector<p_value_class> P_VALUE_VECTOR;
  vector<vector<p_value_class>> P_VALUE_MATRIX;
  vector<hamming_class> HAMMING_VECTOR;
  vector<vector<hamming_class>> HAMMING_MATRIX;
  //vector<vector<vector<hamming_class>>> HAMMING_3D;
  vector<z_test_class> Z_TEST_VECTOR;
  vector<vector<z_test_class>> Z_TEST_MATRIX;
  vector<unsigned int> generic_vector_creation(string);

  void table_creation_horizontal(vector<bed_class> &);
  void table_creation_vertical(vector<bed_class> &);
  void or_ver_kmer_count(string, unordered_map<string, unsigned int> &,
                         unordered_map<string, unsigned int> &);
  void vertical_kmer_count(
      string, map<pair<string, string>, pair<unsigned int, unsigned int>> &,
      unsigned int &);
  void
  select_best(map<pair<string, string>, pair<unsigned int, unsigned int>> &);
  void print_debug_orizzontal();
  void P_VALUE_MATRIX_creation();
  void HAMMING_MATRIX_creation();
  void Z_TEST_MATRIX_creation(vector<bed_class> &);
  ofstream outfile_header(unsigned int);
  ofstream outfile_header_hamming(unsigned int);
  void TopN_sum_and_freq();
  void p_value_parameters_debug_p_val();
  void p_value_parameters_debug_occ();
  void check_kmer_dist();
  void Outfile_PWM_matrices();
  void Outfile_Z_score_values();
  void print_debug_PWM_hamming(ofstream &, unsigned int, unsigned int);
  void print_debug_PWM_hamming_tomtom(ofstream &, unsigned int, unsigned int);
  void print_debug_Z_scores(ofstream &, unsigned int, unsigned int);
  bool find_local_max(double, double, double);
  unsigned int sequences_number_T;

public:
  map_class(vector<bed_class> &GEP, string kmers, string dist) {

    // Reading k-mers in input and saving them into a vector
    kmers_vector = generic_vector_creation(kmers);

    // Reading distance parameters in input and saving them into a vector
    distance_vector = generic_vector_creation(dist);

    // Control if k-mers and distance parameters inserted are equal
    check_kmer_dist();

    // Counting all k-mers occurrences along all the sequences (orizzontal
    // count) --> Maps composed by strings (oligos) and unsigned integers (oligo
    // occurrences)
    table_creation_horizontal(GEP);

    // Counting k-mers occurrences for each position (vertical count) --> Maps
    // (one per position) are composed by pair of strings(oligo + reverse
    // complement) and unsigned integers (oligo occ. + reverse oligo occ.)
    table_creation_vertical(GEP);

    sequences_number_T = GEP.size();

    // Function to create an output file of orizzontal map. Oligos are ranked by
    // their occurrences
    print_debug_orizzontal();

    // Function to call and handle the p_value_class constructor
    P_VALUE_MATRIX_creation();

    // Functions to print the parameters to calculate p_values. For each oligo
    // ranked all its parameters are visualized. Since the ranking can be made
    // following two different orders (p-value or occurrences), the ranked
    // oligos are different and also their parameters. The if/else condition is
    // made to extract correctly the parameters
    if (ordering == "p") {

      p_value_parameters_debug_p_val();
    }

    else {

      p_value_parameters_debug_occ();
    }

    // Calculating the sum of the top -n oligos occurrences and their frequences
    // along the position
    TopN_sum_and_freq();

    // Function to call and handle the hamming_class constructor
    HAMMING_MATRIX_creation();

    // Function to call and handle the z_test_class constructor
    Z_TEST_MATRIX_creation(GEP);

    // Output PWM matrix generated from hamming analysis --> ancora da
    // modificare
    Outfile_PWM_matrices();
    Outfile_Z_score_values();
  }
};

void GEP_path();
void command_line_parser(int, char **);
void display_help();
bool is_file_exist(string fileName, string buf);
void check_input_file();
bool check_palindrome(string, string &);
double check_p_value(double, string);
void RAM_usage();
