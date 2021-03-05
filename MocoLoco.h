#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>
#include <typeinfo> 
#include <sstream>
#include <numeric>
#include "./TwoBit/twobit.h"
#include "./TwoBit/twobit.c"
#include <getopt.h>

using namespace std;

string BED_FILE; 		//initializing const char variarible for Bed_file input reading
int half_length = 150; 		//default half_length 150
string TWOBIT_FILE;	//initializing const char variable for Twobit_file input reading
string JASPAR_FILE;
const unsigned int overhead = 25;
const double pseudoc = 0.01;
bool DS = 1;

class bed_class { //creation public class of bed_class type        

	private:	//field definition

		string chr_coord;
		unsigned int start_coord;
		unsigned int end_coord;
		bool flag;
		string sequence;

		void read_line(string);
		void flag_control(unsigned int, unsigned int);

	public:				//field definition

		bed_class(unsigned int p, string line, TwoBit* tb,unsigned int n_line){

			read_line(line);					//reading bed line
			flag_control(start_coord,end_coord);			//controlling coordinates
			centering_function(start_coord, end_coord, p, overhead);		//centering the coordinates
			extract_seq(tb, n_line);				//extracting the sequence

		}
		string return_sequence(bed_class);
		string return_chr_coord_GEP();
		unsigned int return_start_coord_GEP();
		void centering_function(unsigned int, unsigned int, unsigned int, const unsigned int);
		void extract_seq(TwoBit*, unsigned int);
		string return_chr_coord();
		unsigned int return_start_coord();
		unsigned int return_end_coord();

};

class matrix_class {

	private: //field definition

		string matrix_name;
		string tf_name;
		vector<vector<double>> matrix;
		vector<vector<double>> norm_matrix;
		vector<vector<double>> inverse_norm_matrix;
		vector<vector<double>> matrix_log;
		vector<vector<double>> inverse_matrix_log;
		vector<double> col_sum;		


		void matrix_normalization_pseudoc(vector<vector<double>>, double);
		void matrix_normalization(vector<vector<double>>);
		void matrix_logarithmic(vector<vector<double>>);
		void read_JASPAR(string);
		vector<vector<double>> reverse_matrix(vector<vector<double>>);
		vector<double> find_col_sum(vector<vector<double>>);
		void print_debug_matrix(vector<vector<double>>, string);



	public:
		matrix_class(string JASPAR_FILE){

			read_JASPAR(JASPAR_FILE);
			matrix_normalization_pseudoc(matrix, pseudoc);			//Calling matrix normalization function
			matrix_normalization(norm_matrix);
			matrix_logarithmic(norm_matrix);
			inverse_norm_matrix = reverse_matrix(norm_matrix);
			inverse_matrix_log = reverse_matrix(matrix_log);

		}

		void debug_matrix(matrix_class);
		vector<vector<double>> return_log_matrix();
		vector<vector<double>> return_inverse_log_matrix();

};


class oligo_class{

	private:


		vector<double> oligo_scores;	
		vector<double> o_matrix_mins;	
		vector<double> o_matrix_maxes;
		double min_possible_score;
		double max_possible_score;
		double best_score;
		double best_score_normalized;
		string global_sequence;
		string best_oligo_seq;
		unsigned int local_position;
		string chr_coord_oligo;
		unsigned int start_coord_oligo;
		unsigned int end_coord_oligo;
		char strand;

		void find_minmax(vector<vector<double>>);
		unsigned int find_best_score(vector<double>);
		void find_coordinate(unsigned int, unsigned int, string, unsigned int);
		void find_best_sequence(string, unsigned int, unsigned int);
		void best_score_normalization();

	public:
		oligo_class(vector<vector<double>> matrix, string sequence, string chr_coord_GEP, unsigned int start_coord_GEP, char strand_sign){

			global_sequence = sequence;
			strand = strand_sign;
			find_minmax(matrix);		
			shifting(matrix, sequence, 0);
			local_position = find_best_score(oligo_scores);
			best_score_normalization();
			find_best_sequence(sequence, local_position, matrix[0].size());
			find_coordinate(local_position, matrix[0].size(), chr_coord_GEP, start_coord_GEP);
		}

		void shifting(vector<vector<double>>, string, unsigned int);
		void oligos_vector_debug(oligo_class);
		unsigned int return_start_coord_oligo();
		double return_best_score_normalized();
};

class coordinator_class{ 					//Coordinator class to connect Matrix to Bed and Oligos_vector

	private:

		vector<vector<double>> matrix_log;
		vector<vector<double>> inverse_matrix_log;
		void centering_oligo();
		void best_strand(vector<oligo_class>);


	public:
		vector<oligo_class> oligos_vector;
		vector<bed_class> GEP; 
		void GEP_creation(string, string, vector<bed_class>&);
		void oligos_vector_creation(vector<oligo_class>&, vector<vector<double>>, vector<vector<double>>, vector<bed_class>);
		void print_debug_GEP(vector <bed_class>);

		coordinator_class(){
			GEP_creation(BED_FILE, TWOBIT_FILE, GEP);
			matrix_class M(JASPAR_FILE);
			matrix_log = M.return_log_matrix();
			inverse_matrix_log = M.return_inverse_log_matrix();
			oligos_vector_creation(oligos_vector, matrix_log, inverse_matrix_log, GEP);
			best_strand(oligos_vector);
			centering_oligo();
		}

};

void command_line_parser(int, char **);
void display_help();
bool exist_test0(string);
bool is_file_exist(string fileName, string buf);
