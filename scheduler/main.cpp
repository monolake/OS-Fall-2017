#include <string>
#include "DES.h"

using namespace std;

int main(int argc, char *argv[])
{
    //cout << "Hello world!" << endl;

    string command = argv[1];
    char sched_type = command[2];
    int quantum = 0;
    if (sched_type == 'R' || sched_type == 'P') {
        stringstream number(command.substr(3));
        number >> quantum;
    }
    DES des(sched_type, quantum);
    //DES des('P', 2);
    string filename = argv[2];
    des.readInput(filename);
    string randfile = argv[3];
    des.readRandom(randfile);
    des.simulate();
    //cout << "sched_type " << sched_type << " quantum " << quantum << " filename " << filename << " rand " << randfile << endl;
    return 0;
}

