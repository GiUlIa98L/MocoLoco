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


    
class TfMap {
    
    private:
    string name;
    unordered_map<string, string> tfmap;
    unordered_map<string, string> tfmap1000;
    

    public:
    TfMap(string name) {
        this->name = name;        
    }

    void set_tfmap(string tf2) {
        tfmap[tf2];
    }
    void set_tfmap1000(string tf2) {
        tfmap1000[tf2];
    }

    string getName(){
        return this->name;
    }

    tuple <string, bool> getValue(string key) {
        string coords;
        bool is1000 = false;
        auto it = tfmap1000.find(key);
        if (it != tfmap1000.end()) {
            coords = it->second;
            is1000 = true;
        } else {
            auto it = tfmap.find(key);
            if (it != tfmap.end()) {
                coords = it->second;
            }
        }
        return make_tuple(coords, is1000);
    }

    void updateMap(vector<string> tfSel, string chrR) {
        for (string key : tfSel) {
            auto it = tfmap1000.find(key); 
            if (it != tfmap1000.end())
                it->second.append(chrR);
            else {
                auto it = tfmap.find(key); 
                if (it != tfmap.end())
                    it->second.append(chrR);
            }
        }
    }

    void writefile(string coords, const char* filename) {
        const int len = coords.length();
        const char* arr = coords.c_str();
        FILE* pWrite = fopen(filename, "wb");
        fwrite(&arr[0], 1, len, pWrite);
    }

};