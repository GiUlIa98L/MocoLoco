#include <iostream>
#include <cctype>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <vector>
#include <list>
#include <map>
#include "class2.h"
#include <istream>
#include <ostream> 
#include <bits/stdc++.h>

 
using namespace std;

void tokenize(string const &str, const char delim, vector<string> &out);
void split(string const &str, const char delim, list<string> &tfs);
int countMatches(vector<string> tf1ChRegions, vector<string> tf2ChRegions);
void set_tfs(list<string> &tfs, vector<string> &tfSet);

int main(){
    
    std::time_t inizioEsec = std::time(nullptr);
    map<string, TrascriptionFactor> trascriptionFactors;
    string line;
    ifstream f ("file.txt");
    if (f.is_open()) {
       
        while (getline (f, line) ) {

            vector<string> out;
            list<string> tfs;
            vector<string> tfSet;
            tokenize(line, '\t', out); 
            string TFS;
            TFS = out[3];
            
            split(TFS, ',', tfs);
            set_tfs(tfs, tfSet);

            if (trascriptionFactors.empty()){
                
                trascriptionFactors.insert({tfSet[0], TrascriptionFactor(tfSet)});
                tfSet.erase(tfSet.begin());
            }  

            for (string key : tfSet) { 
                
                auto it = trascriptionFactors.find(key); 
                if (it != trascriptionFactors.end())
                    it->second.updateMap(tfSet);
                else {
                    trascriptionFactors.insert({key, TrascriptionFactor(tfSet)});
                }
            }
        }

    }

    int tfSize = trascriptionFactors.size();
    ofstream MyFile("table3.txt");

    /* for(auto iter = trascriptionFactors.begin(); iter != trascriptionFactors.end(); ++iter){
        //iter->first;
        //ignore value
        TrascriptionFactor tf = iter->second;
        tf.getValue()

    } */
    for(auto it : trascriptionFactors) {
        if (it.first == trascriptionFactors.begin()->first) {
            MyFile << " \t" + it.first;
        } else {
            MyFile << "\t" + it.first;
        }
    }
    MyFile << endl;
    
    
    for(auto it1 : trascriptionFactors) {
        for(auto it2 : trascriptionFactors) {    
            if (it2.first == trascriptionFactors.begin()->first) {
                MyFile << it1.first + "\t" + to_string(it1.second.getValue(it2.first));
            }
            else {           
                MyFile << "\t"  +  to_string(it1.second.getValue(it2.first));
            }                   
        }
        MyFile << endl;
    }

    MyFile.close();          

    std::time_t fineEsec = std::time(nullptr);
    cout << "tempo di esecuzione: " + to_string(fineEsec - inizioEsec) << endl;
    return 0;       
} 

void set_tfs(list<string> &tfs, vector<string> &tfSet) {
    
    for (string x : tfs) { 
        int c = count(tfSet.begin(), tfSet.end(), x);
        if (c==0) {
            tfSet.push_back(x);
        } else {
            continue;
        }
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

void split(string const &str, const char delim, list<string> &tfs){
    // construct a stream from the string
    stringstream ss(str);
 
    string s;
    while (getline(ss, s, delim)) {
        tfs.push_back(s);
    }
}