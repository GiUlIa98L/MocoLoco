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
#include <thread>
#include <vector>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

class TfMap {
    
    private:
    string name;
    unordered_map<string, int> tfmap;
    string Matrix;
    

    public:
    TfMap(string name) {
        this->name = name;        
    }

    void addValue(string tf2, int value) {
        tfmap.insert({tf2, value});
    }

    void addJaspar(string mat){
        this->Matrix = mat;
    }

    bool getValue (string tf2) {
        bool result = false;
        auto it = tfmap.find(tf2);
        if (it != tfmap.end()) {
            result = true;
        }
        return result;
    }

    string getName() {
        return this->name;
    }

    string getJaspar() {
        return this->Matrix;
    }
    
};


