#include <iostream>
#include "VMM.h"

using namespace std;

int main(int argc, char* argv[])
{
    cout << "Hello world!" << endl;


    string filename = "in10";
    VMM vmm(filename);
    vmm.readInput(filename);

    string randfile = "rfile";
    vmm.readRandom(randfile);

    vmm.simulate();
    return 0;
}
