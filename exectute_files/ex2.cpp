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
#include <functional>
#include <tbb/tbb.h>
#include <thread>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
#include <filesystem>
#include "class.h"
#include <dirent.h>
#include <stdexcept>
#include <sys/wait.h>

using namespace std;

// Define a global variable to store the desired number of threads
int desiredThreads = 40; 
//tbb::global_control::active_value(tbb::global_control::max_allowed_parallelism);
// Define a flag to signal the monitoring thread to exit
std::atomic<bool> monitorExitFlag(false);

double getCPULoad();
int calculateDesiredThreads();
void executeMOCO(const std::string& programPath, const std::string& jaspar, const std::string& genome, const std::string& bedFile);
bool fileExists(const std::string& filePath);
void tokenize(string const &str, const char delim, vector<string> &out);
string removeQuotes(string stringVariable);
bool check_number(string str);
std::pair<std::vector<std::string>, std::string> getFilesInDirectory(const std::string& directoryPath, TfMap& tf);
bool empty(std::ifstream& pFile);

int main() {
    string line;
    vector <string> tfs;
    vector <TfMap> tfObj;
    int n=0;
    ifstream f1 ("/home/loig/MocoLoco/exectute_files/motifSimilarity_table.csv");
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
                            if (value <= 0.6 ) { //Similarity treshold
                                
                                tfObj[n].addValue(tf2, value);
                            }
                        }
                    }
                } else {break;}

                n +=1;
            }
        }
    }
    
    std::string str = "g++ -o MOCO -Wall /home/loig/MocoLoco/MocoLoco.cpp -lgsl";
    const char *command = str.c_str();
    //Compiling file
    system(command);
    // Define the path to your program and common arguments
    std::string programPath = "./MOCO"; // Change to the actual path of your program
    std::string genome = "/home/loig/MocoLoco/Genomes/hg38/hg38.2bit";

    for (auto& tf : tfObj){
        string tf1 = "NFYA";
        if(tf1 == tf.getName()){
            string directoryPath = "/home/loig/MocoLoco/remap_hg_bed/" + tf1;
            std::pair<std::vector<std::string>, std::string> result  = getFilesInDirectory(directoryPath, tf);
            std::vector<std::string> bedFiles = result.first;
            std::string jaspar = result.second;
                    // Create a thread-safe queue to hold file indices
            tbb::concurrent_bounded_queue<size_t> fileIndices;

            // Enqueue all the file indices you want to process
            for (size_t i = 0; i < bedFiles.size(); ++i) {
                fileIndices.push(i);
            }

            // Create a monitoring thread
            auto monitorAndAdjustThreads = [&]() {
                while (!monitorExitFlag.load(std::memory_order_acquire)) {
                    // Calculate the desired number of threads
                    int newThreads = calculateDesiredThreads();
                    if (newThreads != desiredThreads) {
                        
                        desiredThreads = newThreads;
                        std::cout << desiredThreads << std::endl;
                        tbb::global_control max_allowed_parallelism_control(tbb::global_control::max_allowed_parallelism, desiredThreads);
                        
                    }

                    // Sleep for a while before the next monitoring cycle
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                }
            };
            std::thread monitorThread(monitorAndAdjustThreads);
            // Custom observer for monitoring and adjusting threads
            

            // Parallel task execution
            tbb::parallel_for(0, desiredThreads, [&](int threadId) {
                size_t fileIndex;
                while (fileIndices.try_pop(fileIndex)) {
                    executeMOCO(programPath, jaspar, genome, bedFiles[fileIndex]);
                }
            });

            // Signal the monitor thread to exit
            monitorExitFlag.store(true, std::memory_order_release);
            //monitorObserver.cancel_wait();

            // Wait for the monitor thread to finish
            monitorThread.join();
            //execute the matrix_sel.cpp
            //system("./matrix_sel");
        }
    }
    return 0; 
}

// Function to calculate CPU usage as a percentage
double getCPULoad() {
    static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;

    // Delay to calculate CPU usage over time
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Read the current CPU statistics from /proc/stat
    std::ifstream statFile("/proc/stat");
    std::string line;
    std::getline(statFile, line);

    // Parse the CPU usage values
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle;
    sscanf(line.c_str(), "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow, &totalSys, &totalIdle);

    // Calculate the CPU usage percentage
    unsigned long long total = totalUser - lastTotalUser + totalUserLow - lastTotalUserLow +
                               totalSys - lastTotalSys;
    unsigned long long totalIdleTime = totalIdle - lastTotalIdle;
    double cpuUsage = 100.0 * (1.0 - (totalIdleTime * 1.0 / total));

    // Update the last values for the next calculation
    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;

    return cpuUsage;
}

// Implement your own logic to calculate the desired number of threads
int calculateDesiredThreads() {
    // Example: Adjust the number of threads based on CPU load
    // Here, we'll use a simple threshold (e.g., 70% CPU usage)
    double cpuUsage = getCPULoad(); // Implement this function to get CPU usage
    std::cout << cpuUsage << std::endl;
    if (cpuUsage < 99) {
        // If CPU usage is high, reduce the number of threads
        return desiredThreads - 1;
    } else {
        // If CPU usage is low, increase the number of threads
        if (desiredThreads<46)
        {
            return desiredThreads + 1;
        } else {
            return desiredThreads;
        }
        
    }
}




void executeMOCO(const std::string& programPath, const std::string& jaspar, const std::string& genome, const std::string& bedFile) {
    // Construct the command arguments for ./MOCO
    std::vector<std::string> arguments = {
        programPath,
        "-b", bedFile,
        "-t", genome,
        "-j", jaspar
    };

    // Join the arguments into a space-separated command string
    std::string command;
    for (const std::string& arg : arguments) {
        command += arg + " ";
    }

    // Execute the command as a child process
    if (system(command.c_str()) == -1) {
        perror("system"); // Handle execution error if needed
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


std::pair<std::vector<std::string>, std::string> getFilesInDirectory(const std::string& directoryPath, TfMap& tf) {
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
                    //select only files for tf2 that has a matrix from class:
                    string file = entry->d_name;
                    vector<string> out;
                    tokenize(file, '_', out);
                    if (out.size()>1) {
                        
                        string tf2 = out[1].erase(out[1].size() - 4);
                        //isolate the tf2 name:
                        if(tf.getValue(tf2)==true) {
                            //check if tf2 name is inside tf1 class
                            files.push_back(directoryPath + "/" + file);
                        }
                    }
                    
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
