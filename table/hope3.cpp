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
    vector <TrascriptionFactor> trascriptionFactors;
    string line;
    ifstream f ("remap2022_crm_macs2_hg38_v1_0.bed");
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
                trascriptionFactors.push_back(TrascriptionFactor(tfSet[0], tfSet));
                for (int x=1; x<tfSet.size(); x++) {
                    trascriptionFactors.push_back(TrascriptionFactor(tfSet[x], tfSet));
                }

            } else {
                for (string x : tfSet) { 
                    int n=0;
                    for(auto& tf : trascriptionFactors){
                        if (x!=tf.getName()) {
                            n = n + 1;
                        } else {
                            if (x==tf.getName()) {
                                tf.updateMap(tfSet);
                                break;
                            }
                        } 
                        if (n==trascriptionFactors.size()) {
                            trascriptionFactors.push_back(TrascriptionFactor(x, tfSet));
                        }                    
                    }
                }
            }

            

        }

        int tfSize = trascriptionFactors.size();
        ofstream MyFile("table3.txt");
        
        for(int i=0; i<(tfSize+1); i++) {
            for(int j=0; j<(tfSize); j++) {
                
                if (i==0) {
                    if (j==0) {
                        MyFile << " \t" + trascriptionFactors[j].getName();
                    } else {
                       MyFile << "\t" + trascriptionFactors[j].getName();
                    }      
                }
                else { 
                    if (j==0) {
                        int value = trascriptionFactors[(i-1)].getValue(trascriptionFactors[j].getName());
                        MyFile << trascriptionFactors[i-1].getName() + "\t" + to_string(value);
                    }
                    else {
                        int value = trascriptionFactors[(i-1)].getValue(trascriptionFactors[j].getName());
                        
                        MyFile << "\t" + to_string(value);
                    }                
                }
            }
            MyFile << endl;
        }
        MyFile.close();        
    }   
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