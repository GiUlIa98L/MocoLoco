#include <iostream>
#include <cctype>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <vector>
#include <istream>
#include <ostream> 
 
using namespace std;

// class TFs {
//     //attributi    
//     //se questa diventa una lista
//     vector <string> genoma;

//     //string GR;

//     //funzioni (metodi)

//     //costruttori (servono per istanziare oggetti della classe)
//     public Tfs()
//     {
//         this.genoma.push_back("");
//     }
//     public void addGR(string gr){
//         this.genoma.push_back(gr);
//         //qua devi allocare la memoria per un record della lista
//         //e metterlo dentro alla lista la stringa genomica

//     }
//     public void getGR(){
//         //stampa la lista o la ritorna (da cambiare il void in lista)
//     }

//     public static void main (string[] args){
        
//         TFs tf1= new tfs();
//         TFs tf2= new tfs();
//         malloc(sizeof(TFs));
//         tf1.addGR("genomax");
//         tf2.addGR("genomax");


//         //quando voglio vedere cosa ce un tf1
//         tf1.getGR();
//     }
// }
    
class TrascriptionFactor {
    
    private:
    string name;
    vector<string> chromosomicRegions;

    public:
    TrascriptionFactor(string name, string chromosomicRegion) {
        this->name = name;
        chromosomicRegions.push_back(chromosomicRegion);
    }

    string getName(){
        return this->name;
    }

    vector<string> getChromosomicRegions(){
        return this->chromosomicRegions;
    }
    
    void addChromosomicRegion(string chrRegion){
        this->chromosomicRegions.push_back(chrRegion);
    }

    

};