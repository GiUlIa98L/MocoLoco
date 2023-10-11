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



int main() {

    string dirPath = "/home/loig/MocoLoco/exectute_files";
    std::cout << "Enter the directory path: ";
    std::getline(std::cin, dirPath);
    std::vector<std::string> files;
    DIR* dir = opendir(dirPath.c_str());
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_REG) { // Check if it's a regular file
                //select the files with the .bedDS.txt at the end
                //tokenize it, TF1 is always in penultima position (len(out)-1)
                string file = entry->d_name;
                std::string suffix = ".bedDS.txt";

                if (file.find("hg") == 0 || (file.size() >= suffix.size() && file.compare(file.size() - suffix.size(), suffix.size(), suffix) == 0)) {
                    files.push_back(file);
                }
            } 
        }
        closedir(dir);
    }
    for(string file : files) {
        string filePathstr = dirPath + "/" + file;
        const char* filePath = filePathstr.c_str();
        // Use std::remove to delete the file
        if (std::remove(filePath) == 0) {
            printf("File %s successfully removed.\n", filePath);
        } else {
            perror("Error removing the file");
        }
    } 
    return 0;   
}