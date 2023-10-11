#include <bits/stdc++.h>
using namespace std;
int main ()
{
    
  
    // Build command to execute.  For example if the input
    // file name is a.cpp, then str holds "gcc -o a.out a.cpp" 
    // Here -o is used to specify executable file name
    string str = "g++ ";
    str = str + " -o MOCO " + "-Wall /home/loig/MocoLoco/MocoLoco.cpp" + " -lgsl";
    const char *command = str.c_str();
  
    //Compiling file
    system(command);
    string cmd = "./MOCO";
    string file = "/home/loig/MocoLoco/remap_hg_bed/TEAD4/TEAD4_ADNP";
    string jaspar = "/home/loig/MocoLoco/Jaspar_2020/MA0809.2.jaspar";
    string args = cmd + " -b " + file + " -k 10 " + "-t /home/loig/MocoLoco/Genomes/hg38/hg38.2bit " + "-j " + jaspar + " -d 1" + " -f 0.003";
    const char *cmdargs = args.c_str();
    //Running file
    system(cmdargs);
  
    return 0;
}