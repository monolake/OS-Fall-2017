#include <iostream>
#include "VMM.h"

using namespace std;

int main(int argc, char* argv[])
{
    //cout << "Hello world!" << endl;

// laptop
//    string filename = "in1";
//    VMM vmm(filename);
//
//    vmm.readInput(filename);
//
//    string randfile = "rfile";
//    vmm.readRandom(randfile);
//
//    vmm.simulate(vmm);

// server
    string algo = argv[2];
    string output = argv[3];
    string num_frame = argv[1];
    string filename = argv[4];
    string randfile = argv[5];


    VMM vmm(filename,algo.at(2), num_frame.substr(2));
    vmm.readInput(filename);
    vmm.readRandom(randfile);
    vmm.simulate(vmm, output.substr(2));

    return 0;
}
