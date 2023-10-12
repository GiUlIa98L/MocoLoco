#include <iostream>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <vector>
#include <list>
#include <map>
#include <istream>
#include <ostream> 
#include <bits/stdc++.h>
#include <algorithm>
#include <random>
#include <chrono>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
#include <filesystem>
#include <dirent.h>
#include "classm.h"


using namespace std;

std::pair<VecFiles, std::vector<string>>  getFilesInDirectory(const std::string& dirPath);
void tokenize(string const &str, const char delim, vector<string> &out);
std::pair<std::vector<int>, std::string> selectKmer( string tf2name, VecFiles& tf1Obj, const string& dirPath, vector<string>& tf2files);
bool transferFiles(const std::string& sourcePath, const std::string &destinationPath);
void deletefiles(vector<string>& hgfiles, string path);
/*int writetable (std::string& dirOutputPath, string tf1);*/

int main() {

    const string directoryPath = "/home/loig/MocoLoco/MocoLoco_output/NFYA";    
    std::pair<VecFiles, std::vector<string>> results= getFilesInDirectory(directoryPath);
    VecFiles tf1Obj = results.first;
    vector<string> hgfiles = results.second;
    deletefiles( hgfiles, directoryPath);
    string  path = "/home/loig/MocoLoco/MocoLoco_output";
    string tf1name = tf1Obj.getTF1name(); // function that return tf1 name and list of file names
    string path_tf1 = path + "/" + tf1name + "/" ;

    if (mkdir(path_tf1.c_str(), 0777) == -1) {
        cout << "Directory " + tf1name + " already exist" << endl;
    } else {
        cout << "Directory " + tf1name + " created" << endl;
    }
    
    
    vector <string> tf2names = tf1Obj.getTF2names();
    std::ofstream outputFile;
    outputFile.open(path_tf1 + "output.txt", std::ios::out);
    outputFile << "TF1: " + tf1name + "\n"; //TF1
    for(std::vector<std::string>::iterator it = tf2names.begin(); it != tf2names.end(); ++it) {
        
        std::string tf2 = *it;
        outputFile << "TF2: " + tf2 + "\n"; //TF2
        vector<string> tf2files;
        std::pair<std::vector<int>, std::string> result  = selectKmer(tf2, tf1Obj, path_tf1, tf2files);
        std::vector <int> positionsv = result.first;
        // Construct the string representation
        std::stringstream ss;
        
        for (size_t i = 0; i < positionsv.size(); ++i) {
            ss << positionsv[i];
            if (i < positionsv.size() - 1) {
                ss << ", ";
            }
        }
        // Convert the stringstream to a string
        std::string vstr = ss.str();
        outputFile << "Positions: " + vstr + "\n";
        outputFile << "HMfile: " + result.second + "\n";

        
        /*//transfer all tf2 files
        for (string file : tf2files) {
            std::string sourcePath = directoryPath + "/" + file;
            std::string destinationPath = path_tf1 + "/" + file;

            if (transferFiles(sourcePath, destinationPath)) {
                std::cout << "File transfer successful!" << std::endl;
            } else {
                std::cout << "File transfer failed." << std::endl;
            }
        }*/
        
    }
    outputFile.close();
    //"Usage: pwm_similarity.R <file_path> <string_argument> <vector_argument> <output_file_path>"
        // Construct the R script command and arguments
    std::string rScriptCommand = "Rscript /home/loig/MocoLoco/exectute_files/pwm_similarity.R";
    std::string fullCommand = rScriptCommand + " " + path_tf1;


    // Execute the script file
    int status = system(fullCommand.c_str());

    if (status != 0) {
        std::cerr << "Error running pwm_similarity.R\n";
        return 1;
    }

}


void tokenize(string const &str, const char delim, vector<string> &out){
    // construct a stream from the string
    stringstream ss(str);
 
    string s;
    while (getline(ss, s, delim)) {
        out.push_back(s);
    }
}


std::pair<VecFiles, std::vector<string>>  getFilesInDirectory(const std::string& dirPath) {
    std::vector<std::string> hgfile;
    VecFiles tf1files = VecFiles();
    DIR* dir = opendir(dirPath.c_str());
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_REG) { // Check if it's a regular file
                //select the files with the .bedDS.txt at the end
                //tokenize it, TF1 is always in penultima position (len(out)-1)
                string file = entry->d_name;
                std::string suffix = ".bedDS.txt";

                if (file.find("hg") == 0) { //filter all the files that start with hg
                    hgfile.push_back(file);
                }

                if (file.size() >= suffix.size() && file.compare(file.size() - suffix.size(), suffix.size(), suffix) == 0) {
                    vector<string>out;
                    tokenize(file, '_', out);
                    string tf1name;
                    if(tf1name.empty()) {
                        string tf1name = out[out.size() - 2];
                        tf1files.insertname(tf1name);
                    }
                    
                    string suffix = ".bedDS.txt";
                    string tf2name = out[out.size() - 1];
                    tf2name.erase(tf2name.size() - suffix.size());
                    string filetype;
                    if (out[1]=="Z") {
                        filetype = "zscore";
                    } else {filetype = "pwm";}

                    if (out[0]=="6-mers") {
                        tf1files.insert6mers(tf2name, filetype, file);
                    } else if (out[0]=="8-mers") {
                        tf1files.insert8mers(tf2name, filetype, file);
                    } else {
                        tf1files.insert10mers(tf2name, filetype, file);
                    } 
                }  
            } 
        }
        closedir(dir);
    }
    return std::make_pair(tf1files, hgfile);
}

std::pair<std::vector<int>, std::string> selectKmer( string tf2name, VecFiles& tf1Obj, const string& dirPath, vector<string>& tf2files) {
    
    
    vector <string> Zfiles;
    vector <string> HMfiles;
    for( int i=0; i<3; i++) {
        vector <string> files;
        if (i==0) {
            files = tf1Obj.get6mers(tf2name);
            if (!files.empty()) {
                HMfiles.push_back(files[0]);
                tf2files.push_back(files[0]);
                
                Zfiles.push_back(files[1]);
                tf2files.push_back(files[1]);
            }            
        } else if (i==1) {
            files = tf1Obj.get8mers(tf2name);
            if (!files.empty()) {
                HMfiles.push_back(files[0]);
                tf2files.push_back(files[0]);
                
                Zfiles.push_back(files[1]);
                tf2files.push_back(files[1]);
            }
        } else {
            files = tf1Obj.get10mers(tf2name);
            if (!files.empty()) {
                HMfiles.push_back(files[0]);
                tf2files.push_back(files[0]);
                
                Zfiles.push_back(files[1]);
                tf2files.push_back(files[1]);
            }     
        }
    }
    int highestValue = 0;
    string HMfilemax;
    vector<int> maxpos;
    for (int i=0; i<HMfiles.size(); i++ ) {
        string Zfile = Zfiles[i];
        string HMfile = HMfiles[i];
        vector<int> positions;
        ifstream f (dirPath + "/" + Zfile);
        if (f.is_open()) {
            string line;
            int n = 1;
            while (getline (f, line) ) {
                //go to the right line of the Z file
                if (n>3) {
                    vector<string> out;
                    tokenize(line, '\t', out);
                    double pval = stod(out[10]);
                    double treshold = 6;
                    if (pval > treshold) {
                        int pos = stoi(out[0]);
                        positions.push_back(pos);
                    }
                }
                n += 1;
            }
        }
        if (positions.size()>highestValue) {
            highestValue = positions.size();
            HMfilemax = dirPath + HMfile;
            maxpos = positions;
        }
    }
    
    return std::make_pair(maxpos, HMfilemax);
}


bool transferFiles(const std::string& sourcePath, const std::string& destinationPath) {
    try {
        // Open the source file for reading
        std::ifstream sourceFile(sourcePath, std::ios::binary);
        if (!sourceFile) {
            std::cerr << "Error opening source file: " << sourcePath << std::endl;
            return false;
        }

        // Open the destination file for writing
        std::ofstream destinationFile(destinationPath, std::ios::binary);
        if (!destinationFile) {
            std::cerr << "Error opening destination file: " << destinationPath << std::endl;
            return false;
        }

        // Transfer the file contents
        destinationFile << sourceFile.rdbuf();

        // Close the files
        sourceFile.close();
        destinationFile.close();

        // Remove the source file after successful transfer
        std::remove(sourcePath.c_str());

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error during file transfer: " << e.what() << std::endl;
        return false;
    }
}

void deletefiles(vector<string>& hgfiles, string path) {

    for(string file : hgfiles) {
        string filePathstr = path + "/" + file;
        const char* filePath = filePathstr.c_str();
        // Use std::remove to delete the file
        if (std::remove(filePath) == 0) {
            printf("File %s successfully removed.\n", filePath);
        } else {
            perror("Error removing the file");
        }
    }    
}
