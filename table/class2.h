#include <iostream>
#include <cctype>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <vector>
#include <istream>
#include <ostream> 
#include <map>
#include <unordered_map>
 
using namespace std;


    
class TrascriptionFactor {
    
    private:
    string name;
    unordered_map<string, int> matches;
    

    public:
    TrascriptionFactor(string name, vector<string> tfSet) {
        this->name = name;
        createMap(tfSet);
        
    }

    string getName(){
        return this->name;
    }

    int getValue(string key) {
        auto it = matches.find(key);
        if (it != matches.end())
            return it->second;
        else {
            return 0;
        }
    }

    void createMap(vector<string> tfSet) {
        for (string key : tfSet) {
            matches.insert({key, 1});
        }
    }

    void updateMap(vector<string> tfSet) {
        for (string key : tfSet) {
            auto it = matches.find(key); 
            if (it != matches.end())
                it->second++;
            else {
                matches.insert({key, 1});
            }
        }
    }


    

};