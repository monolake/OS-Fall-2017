#include <iostream>
#include "IOsched.h"
using namespace std;

int main(int argc, char* argv[])
{
// laptop
/*
    cout << "Hello world!" << endl;
    //laptop
    string filename = "input9";

    IOsched sched;

    sched.readInput(filename);
    sched.simulate(sched);
    return 0;
*/

// server

    string schedalg = argv[1];
    string filename = argv[2];
    IOsched sched(schedalg.at(2));

    sched.readInput(filename);
    sched.simulate(sched);

    return 0;
}
