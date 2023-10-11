#include <iostream>
#include <cctype>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <vector>
#include <list>
#include <map>
#include "class.h"
#include <istream>
#include <ostream> 
#include <bits/stdc++.h>
#include <algorithm>
#include <random>
#include <chrono>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>

//parser variables
string TABLE_FILE;
string BED_FILE;
int min_size = 500;
int max_size = 1000;
string TF;
string path;
 
using namespace std;

unsigned seed = chrono::system_clock::now().time_since_epoch().count();
void objectsconstruction(vector <TfMap> &tfObjs, vector <string> &tfs, vector<string> &out, string &tf1);
void tokenize(string const &str, const char delim, vector<string> &out);
int countMatches(vector<string> tf1ChRegions, vector<string> tf2ChRegions);
vector<string> select_tfs(vector<string> tfs, string line);
string randomShuffle(vector <string> &coordsVec, unsigned seed);
void display_help();
void command_line_parser(int argc, char **argv);

int main(int argc, char *argv[]){

    if (argc == 1) {
        display_help();
    }

    command_line_parser(argc, argv);
        
    std::time_t inizioEsec = std::time(nullptr);
    vector <TfMap> tfObjs;
    vector<string> tfs;
    string line;
    ifstream f1 (TABLE_FILE);
    if (f1.is_open()) {
       
        while (getline (f1, line) ) {

            vector<string> out;
            tokenize(line, '\t', out); 
            
            if (tfs.empty()==true)  {
                for (int i=0; i<out.size(); i++) {
                    tfs.push_back(out[i]);
                }
                if (TF.empty()==false) {
                    bool exist;
                    exist = find(tfs.begin(), tfs.end(), TF) != tfs.end();
                    if (exist == false) {
                        cerr << "ERROR: the given tf doesn't exist in the table\n" << endl;
                        display_help();
                        exit(1);
                    }
                }
            } else {
                string tf1 = out[0];
                if(TF.empty()==false) {
                    if(tf1==TF) {
                        objectsconstruction( tfObjs, tfs, out, tf1);
                    }
                } else {
                    objectsconstruction( tfObjs, tfs, out, tf1);
                }
                
                
            }
        }
    }
    ifstream f2 (BED_FILE);
    if (f2.is_open()) {
       
        while (getline (f2, line) ) {

            vector<string> out;
            tokenize(line, '\t', out);
            vector <string> tfSelected = select_tfs( tfs, out[3]);
            string coords = out[0] + "\t" + out[6] + "\t" + out[7] + "\n";

            if (tfSelected.size()>=2) {

                for(string tfLine : tfSelected) {

                    for(TfMap &tf : tfObjs) {
                        
                        if(tf.getName()==tfLine) {
                            tf.updateMap(tfSelected, coords);
                            break;
                        }
                        
                    }
                }
            }
        }
    }

    for(TfMap &tf1 : tfObjs) {
        //create directory
        string tf1name = tf1.getName();
        string path_tf1 = path + "/" + tf1name + "/" ;

        if (mkdir(path_tf1.c_str(), 0777) == -1) {
            cout << "Directory " + tf1name + " already exist" << endl;
        } else {
                cout << "Directory " + tf1name + " created" << endl;
            }

        for(string tf2 : tfs) {
            if(tf1name!=tf2){
                string filename =  path_tf1 + tf1name + "_" + tf2;
                string coords;
                bool is1000;
                tie(coords, is1000) = tf1.getValue(tf2);
                if(coords.empty()==false){
                    if(is1000==true){
                    vector <string> coordsVec;
                    tokenize(coords, '\n', coordsVec);
                    string coordShuffled = randomShuffle( coordsVec, seed);
                    tf1.writefile(coordShuffled, filename.c_str());
                    
                    } else {
                        tf1.writefile(coords, filename.c_str());
                    }
                }
                
            }
        }
    } 
} 

vector<string> select_tfs(vector<string> tfs, string line) {
    
    vector<string> tfSel;
    vector<string> tfsInLine;
    tokenize(line, ',', tfsInLine);


        for (string tf : tfs) { 
            if (find(tfsInLine.begin(), tfsInLine.end(), tf) != tfsInLine.end()) {
                tfSel.push_back(tf);
            }
            
        }
    return tfSel;
}




void tokenize(string const &str, const char delim, vector<string> &out){
    // construct a stream from the string
    stringstream ss(str);
 
    string s;
    while (getline(ss, s, delim)) {
        out.push_back(s);
    }
}

void objectsconstruction(vector <TfMap> &tfObjs, vector <string> &tfs, vector<string> &out, string &tf1) {
    TfMap tfobj = TfMap(tf1);
        for (int j=1; j<out.size(); j++) {
            string tf2 = tfs[j-1];
            if (tf1!=tf2){
                int chRnum = stoi(out[j]);
                if (chRnum >= min_size && chRnum <= max_size) {
                    tfobj.set_tfmap(tf2);
                           
                } else if (chRnum > max_size) {
                    tfobj.set_tfmap1000(tf2);
                            
                } else if (chRnum < min_size) { continue;}
                        
            }
        }
    tfObjs.push_back(tfobj);
}

void split(string const &str, const char delim, list<string> &tfs){
    // construct a stream from the string
    stringstream ss(str);
 
    string s;
    while (getline(ss, s, delim)) {
        tfs.push_back(s);
    }
}


string randomShuffle(vector <string> &coordsVec, unsigned seed) {
    shuffle(coordsVec.begin(), coordsVec.end(), default_random_engine(seed));
    string coordShuffled;
    for (int i=0; i<max_size; i++) {
        coordShuffled.append(coordsVec[i] + "\n");
    }
    return coordShuffled;
}

void display_help() {
  cerr << "\n --help || -h show this message\n";
  cerr << "\n --table || -t <table_file>: input table file\n";
  cerr << "\n --bed || -b <file_bed>: input bed file\n";
  cerr << "\n --max_number || -m : max number of lines in the output file, the default is 1000\n";
  cerr << "\n --min_number || -l: lowest number of lines in the output file, the default is 500\n";
  cerr << "\n --option || -o: <transcription factor>: select only one row (tf) from the table to generate files\n";
  cerr << "\n --path || -p : path where will be located the output files\n";
  exit(EXIT_SUCCESS);
}

bool is_file_exist(string fileName,
                   string buf) { // Input files existence control

  struct stat check;
  int regular_check, existing_check;
  const char *C_fileName = fileName.c_str();
  existing_check = stat(C_fileName, &check);

  regular_check = S_ISREG(check.st_mode);

  if (regular_check == 0 || existing_check != 0) {
    cerr << "ERROR: " << buf << " file does not exist!\n\n";
    display_help();
    exit(1);
  }
  return 0;
}

void check_input_file() {
    if (TABLE_FILE.empty() || BED_FILE.empty() ) {
        cerr << "FATAL ERROR: some arguments needed \n" << endl;
        display_help();
        exit(1);
    }
}

void command_line_parser(int argc, char **argv) {

  const char *const short_opts = "ht:b:m:l:o:p:";

  // Specifying the expected options
  const option long_opts[] = {
      {"help", no_argument, nullptr, 'h'},
      {"table", required_argument, nullptr, 't'},
      {"bed", required_argument, nullptr, 'b'},
      {"max_number", required_argument, nullptr, 'm'},
      {"min_number", required_argument, nullptr, 'l'},
      {"option", required_argument, nullptr, 'o'},
      {"path", required_argument, nullptr, 'p'},
      {nullptr, no_argument, nullptr, 0}};


  while (true) {
    const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

    if (-1 == opt)
      break;

    switch (opt) {
    case 'h':
      display_help();
      break;
    case 't':
      TABLE_FILE = string(optarg);
      is_file_exist(TABLE_FILE, "--table || -t");
      break;
    case 'b':
      BED_FILE = string(optarg);
      is_file_exist(BED_FILE, "--bed || -b");
      break;
    case 'm':
      max_size = stoi(optarg);
      break;
    case 'l':
      min_size = stoi(optarg);
      break;
    case 'o':
      TF = string(optarg);
      break;
    case 'p':
      path = string(optarg);
      break;
    case '?': // Unrecognized option
    default:
      display_help();
      break;
    }
  }
  check_input_file();
}




