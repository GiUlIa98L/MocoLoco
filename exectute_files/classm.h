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

using namespace std;

class VecFiles {
    private:
    string tf1name;
    unordered_map <string, std::map<std::string, string>> files6mers;
    unordered_map <string, std::map<std::string, string>> files8mers;
    unordered_map <string, std::map<std::string, string>> files10mers;

    public:

    VecFiles() {}

    void insertname (string tf1) {
        this->tf1name = tf1;
    }
    void insert6mers(string tf2, string filetype, string originalfile) {
        files6mers[tf2][filetype]=originalfile;
    }
    void insert8mers(string tf2, string filetype, string originalfile) {
        files8mers[tf2][filetype]=originalfile;
    }
    void insert10mers(string tf2, string filetype, string originalfile) {
        files10mers[tf2][filetype]=originalfile;
    }
    vector<string> get6mers(string tf2name) {
        vector<string> result;
        auto it = files6mers.find(tf2name);
        if (it != files6mers.end()) {
            map <string, string> filetypes = it->second;
            string type1 = filetypes["pwm"];
            string type2 = filetypes["zscore"];
            result = { type1, type2};
        }
        return result;
    }

    vector<string> get8mers(string tf2name) {

     //find the two files associated with each tf2 and gives them in this order: pwm, zscores   
        vector<string> result;
        auto it = files8mers.find(tf2name);
        if (it != files8mers.end()) {
            map <string, string> filetypes = it->second;
            string type1 = filetypes["pwm"];
            string type2 = filetypes["zscore"];
            result = { type1, type2};
        }
        return result;
    }

    vector<string> get10mers(string tf2name) {
        vector<string> result;
        auto it = files10mers.find(tf2name);
        if (it != files10mers.end()) {
            map <string, string> filetypes = it->second;
            string type1 = filetypes["pwm"];
            string type2 = filetypes["zscore"];
            result = { type1, type2};
        }
        return result;
    }
    
    string getTF1name() {
        return this->tf1name;
    }
    vector<string> getTF2names() {
        vector<string> tf2names;
        for (const auto& pair : files6mers) {
            tf2names.push_back(pair.first);
        }
        return tf2names;
    }

};