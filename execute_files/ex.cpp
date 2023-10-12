#include <iostream>
#include <unistd.h>
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
#include "class.h"
#include <dirent.h>
#include <stdexcept>
#include <sys/wait.h>

using namespace std;

void executeChildProcess(const std::string& programPath, const std::vector<std::string>& arguments);
bool fileExists(const std::string& filePath);
void tokenize(string const &str, const char delim, vector<string> &out);
string removeQuotes(string stringVariable);
bool check_number(string str);
std::pair<std::vector<std::string>, std::string> getFilesInDirectory(const std::string& directoryPath);
bool empty(std::ifstream& pFile);

int main() {
    string line;
    vector <string> tfs;
    vector <TfMap> tfObj;
    int n=0;
    ifstream f1 ("motifSimilarity_table.csv");
    if (f1.is_open()) {
       
        while (getline (f1, line) ) {
            vector<string> out;
            tokenize(line, '\t', out);
            if (tfs.empty()==true)  {
                for (int i=0; i<out.size(); i++) {
                        string tf = removeQuotes(out[i]);
                        tfs.push_back(tf);
                        tfObj.push_back(TfMap(tf));
                    }
            } else {

                string tf1 = removeQuotes(out[0]);
                bool exist;
                exist = find(tfs.begin(), tfs.end(), tf1) != tfs.end();
                if (exist == true) {
                    for (int i=1; i<out.size(); i++) {
                        if (check_number(out[i])==true){
                            double value = stod(out[i]);
                            string tf2 =tfs[i-1];
                            if (value < 0.3 ) { //Similarity treshold
                                
                                tfObj[n].addValue(tf2, value);
                            }
                        }
                    }
                } else {break;}

                n +=1;
            }
        }
    }
    
    

    
    string directoryPath = "/home/loig/MocoLoco/remap_hg_bed/TEAD4";
    std::pair<std::vector<std::string>, std::string> result  = getFilesInDirectory(directoryPath);
    std::vector<std::string> bedfiles = result.first;
    std::string jaspar = result.second;
    

    
    std::string str = "g++ -o MOCO -Wall /home/loig/MocoLoco/MocoLoco.cpp -lgsl";
    const char *command = str.c_str();
    //Compiling file
    system(command);
    // Define the path to your program and common arguments
    std::string programPath = "./MOCO"; // Change to the actual path of your program
    std::string genome = "/home/loig/MocoLoco/Genomes/hg38/hg38.2bit";

    vector< string > bedFiles10;
    int maxConcurrentExecutions = 20;
    int numConcurrentExecutions = 0;

    for (auto file: bedfiles) {
        
        vector<string> out;
        tokenize(file, '/', out);
        vector<string> out2;
        tokenize(out[6], '_', out2);
        if (out.size()>1) {
            string tf1 = out2[0];
            string tf2 = out2[1].erase(out2[1].size() - 4);
            for (auto& tf : tfObj){

                if(tf1 == tf.getName()){
                    
                    if (tf.getValue(tf2)==true) {
                
                        bedFiles10.push_back(file);
                        numConcurrentExecutions++;

                        if (numConcurrentExecutions >= maxConcurrentExecutions) {
                            // Create child processes and execute the program in parallel
                            std::vector<pid_t> childPids;
                            for (const std::string& bedFile : bedFiles10) {
                                pid_t pid = fork();
                                if (pid == -1) {
                                    perror("fork");
                                    exit(EXIT_FAILURE);
                                } else if (pid == 0) {
                                    // This is the child process
                                    executeChildProcess(programPath, {"-b", bedFile, "-t", genome, "-j", jaspar});
                                    // The child process will never reach here unless there is an execution error
                                } else {
                                    // This is the parent process, store the child's PID
                                    childPids.push_back(pid);
                                }
                            }
                            cout << childPids.size() << endl;
                            // Wait for all child processes to finish
                            for (pid_t pid : childPids) {
                                int status;
                                if (waitpid(pid, &status, 0) == -1) {
                                    perror("waitpid");
                                    exit(EXIT_FAILURE);
                                }
                                if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                                    std::cerr << "Child process failed with an error." << std::endl;
                                    // Handle the error if needed
                                }
                            }

                            // Reset counters and clear the list of bedFiles
                            numConcurrentExecutions = 0;
                            // After bedFiles.clear();
                            std::cout << "bedFiles size after clear: " << bedFiles10.size() << std::endl; // Debugging output

                            // Ensure it's cleared
                            bedFiles10.clear();
                            std::cout << "bedFiles size after second clear: " << bedFiles10.size() << std::endl; // Debugging output

                        }
                    }
                }
            }
        }    
    }
    // Check if there are any remaining bedFiles to process
if (!bedFiles10.empty()) {
    // Create child processes for the remaining bedFiles
    std::vector<pid_t> childPids;
    for (const std::string& bedFile : bedFiles10) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // This is the child process
            executeChildProcess(programPath, {"-b", bedFile, "-t", genome, "-j", jaspar});
            // The child process will never reach here unless there is an execution error
        } else {
            // This is the parent process, store the child's PID
            childPids.push_back(pid);
        }
    }

    // Wait for all child processes to finish
    for (pid_t pid : childPids) {
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            std::cerr << "Child process failed with an error." << std::endl;
            // Handle the error if needed
        }
    }
}
//execute the matrix_sel.cpp
system("./matrix_sel");
    
    
} 

void executeChildProcess(const std::string& programPath, const std::vector<std::string>& arguments) {
    // Convert argument vector to char* array
    std::vector<const char*> args;
    args.push_back(programPath.c_str()); // First argument is the program itself
    for (const std::string& arg : arguments) {
        args.push_back(arg.c_str());
    }
    args.push_back(nullptr); // Last argument must be nullptr

   

    // Execute the program as a child process
    if (execvp(programPath.c_str(), const_cast<char* const*>(args.data())) == -1) {
        perror("execvp"); // Handle execution error if needed
        exit(EXIT_FAILURE);
    }
}

    
bool fileExists(const std::string& filePath) {
    struct stat buffer;
    return (stat(filePath.c_str(), &buffer) == 0);
}
    
void tokenize(string const &str, const char delim, vector<string> &out){
    // construct a stream from the string
    stringstream ss(str);
 
    string s;
    while (getline(ss, s, delim)) {
        out.push_back(s);
    }
}


string removeQuotes(string stringVariable) {
    stringVariable.erase(
    std::remove(stringVariable.begin(), stringVariable.end(), '\"'), 
    stringVariable.end());
    return stringVariable;
}

bool check_number(string str) {
    bool result = false;
    int n =0;
    for(int i = 0 ; i < str.length(); i++) {
        if (isdigit(str[i]) == false) {
            n += 0;
        } else { n += 1;}
    }
    if (n>0) {result = true;}
    return result;
}


std::pair<std::vector<std::string>, std::string> getFilesInDirectory(const std::string& directoryPath) {
    std::vector<std::string> files;
    string jaspar;
    DIR* dir = opendir(directoryPath.c_str());
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_REG) { // Check if it's a regular file
                string jasparfile = entry->d_name;
                if(jasparfile=="aaa_matrix.bed") {
                    ifstream f (directoryPath + "/" + entry->d_name);
                    if (f.is_open()) { 
                        if (empty(f)==false) { //check if file is empty
                            string line;
                            getline (f, line);
                            jaspar = "/home/loig/MocoLoco/Jaspar_2020/" + line + ".jaspar";
                            if (fileExists(jaspar)==false) {//check if file exist
                                std::cerr << "Error: The Jaspar file doesn't exists." << std::endl;
                            }
                        }
                    }
                } else {
                    files.push_back(directoryPath + "/" + entry->d_name);
                } 
            }
        }
        closedir(dir);
    }
    return std::make_pair(files, jaspar);
}

bool empty(std::ifstream& pFile)
{
    return pFile.peek() == std::ifstream::traits_type::eof();
}
