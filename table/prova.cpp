#include <iostream>
#include <cctype>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <vector>
 
using namespace std;

int main()
{
    
    string line;
    ifstream f ("file.txt");
    if (f.is_open()) {
        while (getline (f, line) ) 
        {

            string word;
            string GR;
            int n = line.length();
            char char_array[n + 1];
            strcpy(char_array, line.c_str());
            vector<string> TFs;
            
            
            int p=2;
            for (int j=0; j<4; j++) {
                for (int i=p; i<n; i++)
                {
                    p += 1;
                    if (!isspace(char_array[i]))   {
                          word = word + char_array[i];
                    } else  { 
                        if (isspace(char_array[i+1])) {
                            p += 1;
                        }
                        break;
                    }
                        
                }
                if (j<2) {
                    GR = GR + word;
                    word = "";
                } else if (j==2) {
                    GR = GR + word;
                    word = "";
                                        
                } else {                                       
                    string delimiter = ",";
                    size_t pos = 0;
                    string token;
                    while ((pos = word.find(delimiter)) != string::npos) {
                            token = word.substr(0, pos);
                            TFs.push_back( token );
                            word.erase(0, pos + delimiter.length());
                            }
                            TFs.push_back( word );                                     
                }
            }
            cout << TFs.size();
            for (int k=0; k< TFs.size(); k++)
            cout << GR << " " << TFs[k] << "\n";
            }       
            
            }
    return 0;
}