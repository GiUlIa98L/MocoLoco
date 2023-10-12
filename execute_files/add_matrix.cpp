#include <iostream>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <istream>
#include <ostream>
#include <vector>
#include <unordered_map>

using namespace std;

void tokenize(string const &str, const char delim, vector<string> &out){
    // construct a stream from the string
    stringstream ss(str);
 
    string s;
    while (getline(ss, s, delim)) {
        out.push_back(s);
    }
}

void writeFileInSubdirectories(const std::string& rootPath, const std::string& fileName, unordered_map<string, string> &tfs) {
    DIR* dir;
    struct dirent* entry;

    dir = opendir(rootPath.c_str());

    if (dir == nullptr) {
        std::cerr << "Failed to open directory: " << rootPath << std::endl;
        return;
    }

    while ((entry = readdir(dir)) != nullptr) {
        std::string entryName(entry->d_name);

        if (entryName != "." && entryName != "..") {
            std::string subdirectoryPath = rootPath + "/" + entryName;
            

            std::ofstream file(subdirectoryPath + "/" + fileName);
            if (file) {
                string matrix;
                auto it = tfs.find(entryName);
                if (it != tfs.end()) {
                    matrix = it->second;
                    file << matrix << std::endl;
                    file.close();
                } else {
                    std::cerr << "Failed to create file: " << subdirectoryPath << "/" << fileName << std::endl;
                }

                
            } else {
                std::cerr << "Failed to create file: " << subdirectoryPath << "/" << fileName << std::endl;
            }
        }
    }

    closedir(dir);
}



int main() {

    //add matrix file with matrix ID inside each TF folder
    
    unordered_map <string, string> tfs;
    string line;
    ifstream f1 ("matrix.txt");
    if (f1.is_open()) {
       
        while (getline (f1, line) ) {
            vector<string> out;
            tokenize(line, '\t', out);
            string tfname = out[2].erase(out[2].size() - 1);
            string mat = out[1];
            tfs.insert({tfname, mat});
            
        }
    }

    std::string rootPath = "/home/loig/MocoLoco/remap_hg_bed";  // Replace "." with the desired root directory

    std::string fileName = "aaa_matrix";

    writeFileInSubdirectories(rootPath, fileName, tfs);

    return 0;
}
