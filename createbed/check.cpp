#include <iostream>
#include <cctype>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <vector>
#include <istream>
#include <ostream> 
#include <unordered_map>
#include <dirent.h>
 
using namespace std;

class CheckTF {
    private:
    string tf1;
    unordered_map <string, int> tf2s;
    public:
    CheckTF(string tf1name) {
        this->tf1 = tf1name;
    }

    void addTf( string tf2, int chrR)  {
        tf2s.insert(std::make_pair(tf2, chrR));
    }

    unordered_map <string, int> getMap() {
        return this->tf2s;
    }   

    string getTf1() {
        return this->tf1;
    }
};

void tokenize(string const &str, const char delim, vector<string> &out){
    // construct a stream from the string
    stringstream ss(str);
 
    string s;
    while (getline(ss, s, delim)) {
        out.push_back(s);
    }
}

std::pair<int, std::unordered_map<std::string, int>> countBedFiles(const char* directoryPath) {
    unordered_map <string, int> tfChrRcount;
    int count = 0;
    DIR* dir;
    struct dirent* entry;

    // Open the directory
    dir = opendir(directoryPath);

    if (dir == NULL) {
        throw std::runtime_error("Error opening directory.");
    }

    // Loop through directory entries
    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_REG) { // Check if it's a regular file
            const char* fileName = entry->d_name;
            size_t len = strlen(fileName);

            // Check if the file ends with ".bed"
            if (len >= 4 && strcmp(fileName + len - 4, ".bed") == 0) {
                count++;
                vector<string> out;
                tokenize(fileName, '_', out);
                string tf2 = out[1];
                tf2.erase(tf2.size() - 4);

                string filepath = std::string(directoryPath) +"/" + fileName;
                std::ifstream file(filepath);

                if (!file.is_open()) {
                    throw std::runtime_error("Error opening the file.");
                }

                int lineCount = 0;
                std::string line;

                while (std::getline(file, line)) {
                    lineCount++;
                }

                file.close();
                tfChrRcount.insert(std::make_pair(tf2, lineCount));

            }
        }
    }

    // Close the directory
    closedir(dir);

    return  std::make_pair(count, tfChrRcount);
}


int main() {
    vector <CheckTF> tfObjs;
    vector<string> tfs;
    string line;
    ifstream f1 ("/home/loig/MocoLoco/createbed/table_filtered.txt");
    if (f1.is_open()) {
       
        while (getline (f1, line) ) {

            vector<string> out;
            tokenize(line, '\t', out); 
            
            if (tfs.empty()==true)  {
                for (int i=0; i<out.size(); i++) {
                    tfs.push_back(out[i]);
                }
            } else {
                string tf1name = out[0];
                CheckTF tfObj = CheckTF(tf1name);
                for(int i = 0; i< tfs.size(); i++) {
                    string tf2name = tfs[i];
                    if (tf1name!=tf2name) {
                        int chrRegionsN = stoi(out[i+1]);
                        if (chrRegionsN >= 500 && chrRegionsN <= 1000) {
                            tfObj.addTf(tf2name, chrRegionsN);                            
                        } else if (chrRegionsN > 1000) {
                            chrRegionsN = 1000; 
                            tfObj.addTf(tf2name, chrRegionsN);
                        }
                    }
                }
                tfObjs.push_back(tfObj);
            }
        }
    }
    
    std::ofstream outputFile("/home/loig/MocoLoco/createbed/check_output.txt");

    // Check if the file stream was opened successfully
    if (!outputFile.is_open()) {
        std::cerr << "Error opening the file." << std::endl;
        return 1;
    }
    for(auto &tfobj : tfObjs ) {
        string directoryPath = "/home/loig/MocoLoco/remap_hg_bed/";
        string tf1 = tfobj.getTf1();
        unordered_map <string, int> table_tfs_chrRs = tfobj.getMap();
        directoryPath = directoryPath + tf1;
        const char* directory = directoryPath.c_str();
        std::pair<int, std::unordered_map<std::string, int>> results = countBedFiles(directory);
        int num_folder_files = results.first;
        std::unordered_map<std::string, int> folder_files = results.second;
        int sumTfs = table_tfs_chrRs.size();
        if (sumTfs == num_folder_files) {
            outputFile << "the directory of: " + tf1+ " contains the right number of files: " << num_folder_files << endl;
        } else {
            outputFile << "the directory of: " + tf1+ " contains : " << num_folder_files << "but should contain: " << sumTfs<< endl;
        }
        for (const auto& pair : table_tfs_chrRs) {
            const std::string& tf2 = pair.first;
            int value1 = pair.second;

            // Check if the key exists in map2
            if (folder_files.find(tf2) != folder_files.end()) {
                double value2 = folder_files[tf2];
                outputFile << "TF2: " << tf2 << ", number of chr Regions in table : " << value1 << ", number of chr Regions in file : " << value2 << std::endl;
            } else {
                outputFile << "TF2: " << tf2 << ", number of chr Regions in table : " << value1 << endl;
            }
        }
        
    }
    outputFile.close();
}