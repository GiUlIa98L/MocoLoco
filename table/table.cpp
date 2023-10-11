#include <iostream>
#include <string>
#include <iterator>
#include <fstream>
#include <vector>

using namespace std;

int main()
{
    string line;
    ifstream F ("bed.bed");
    if (F.is_open()) {/*ok proceed*/
    while ( getline(F, line))
    {
        /*splittare per spazio*/
    }}
}