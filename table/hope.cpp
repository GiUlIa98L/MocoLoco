#include <iostream>
#include <cctype>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <vector>
#include <list>
#include "class.h"
#include <istream>
#include <ostream> 
#include <bits/stdc++.h>

 
using namespace std;

void tokenize(string const &str, const char delim, vector<string> &out);
void split(string const &str, const char delim, list<string> &tfs);
int countMatches(vector<string> tf1ChRegions, vector<string> tf2ChRegions);
void set_tfs(list<string> &tfs, vector<string> &tfSet);

int main(){
    
    vector <TrascriptionFactor> trascriptionFactors;
    string line;
    ifstream f ("file.txt");
    if (f.is_open()) {
       
        while (getline (f, line) ) {

            vector<string> out;
            list<string> tfs;
            vector<string> tfSet;
            tokenize(line, '\t', out); 
            string chrRegion, TFS;
            chrRegion = out[0] + out[1] + out[2];
            TFS = out[3];
            
            split(TFS, ',', tfs);
            set_tfs(tfs, tfSet);
                        
            for (string x : tfSet) { 
                
                if (trascriptionFactors.empty()==false){
                    int n=0;
                    for(auto& tf : trascriptionFactors){
                        
                        if (x!=tf.getName()) {
                            n = n + 1;
                        } else {
                            if (x==tf.getName()) {
                                tf.addChromosomicRegion(chrRegion);
                                break;
                                 }
                             } if (n==trascriptionFactors.size()) {
                        trascriptionFactors.push_back(TrascriptionFactor(x, chrRegion));
                         }                    
                    }
                 }else {
                         trascriptionFactors.push_back(TrascriptionFactor(x, chrRegion));
                         }
             }
        }
        int tfSize = trascriptionFactors.size();
        int intersection[tfSize][tfSize] = {};
        
        for(int i=0; i<tfSize; i++) {
            TrascriptionFactor tf1 = trascriptionFactors[i];
            for(int j=i; j<tfSize; j++) {
               
                if(i==j){
                    int chromosomicRegionNumber = tf1.getChromosomicRegions().size();
                    intersection[i][j] = chromosomicRegionNumber;
                   
                } else {
                    TrascriptionFactor tf2 = trascriptionFactors[j];
                    vector<string> copy_tf2= tf2.getChromosomicRegions();
                    int num_matches= countMatches(tf1.getChromosomicRegions(), copy_tf2);
                    intersection[i][j] = num_matches;
                    intersection[j][i] = num_matches;
                    
                }
            }
            
        }

        ofstream MyFile("table.txt");
        
        for(int i=0; i<(tfSize+1); i++) {
            

            for(int j=0; j<(tfSize+1); j++) {
                
                if (i==0) {
                    if (j==0) {
                        MyFile << " \t";
                    } else {
                      
                       MyFile << "\t" + trascriptionFactors[j-1].getName();
                    }      
                }

                else { 
                    if (j==0) {
                         
                        MyFile << trascriptionFactors[i-1].getName() + "\t";
                    }
                    else {
                        /*TrascriptionFactor tf1 = trascriptionFactors[i-1];
                        TrascriptionFactor tf2 = trascriptionFactors[j-1];
                        string internum = to_string(countMatches(tf1.getChromosomicRegions(), tf2.getChromosomicRegions()));*/
                        string internum= to_string(intersection[i-1][j-1]);
                        MyFile << "\t"  + internum; 
                    }
                }                
            }
            MyFile << endl;
        }
        MyFile.close();


    } 

    return 0;    
}       
    //no funge
int countMatches(vector<string> tf1ChRegions, vector<string> copy_tf2){
    
    int count=0;
    int last=-1;
    for(int i=0; i<tf1ChRegions.size(); i++) {
       
        for(int j=0; j<copy_tf2.size(); j++) {
            
            if(tf1ChRegions[i]==copy_tf2[j]) {
                count = count+1;
                int iter = copy_tf2.begin()+j;
                copy_tf2.erase(j);
                break;
            }
        }
    }
    return count;
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



void tokenize(string const &str, const char delim,
            vector<string> &out)
{
    // construct a stream from the string
    stringstream ss(str);
 
    string s;
    while (getline(ss, s, delim)) {
        out.push_back(s);
    }
}

void split(string const &str, const char delim,
            list<string> &tfs)
{
    // construct a stream from the string
    stringstream ss(str);
 
    string s;
    while (getline(ss, s, delim)) {
        tfs.push_back(s);
    }
}