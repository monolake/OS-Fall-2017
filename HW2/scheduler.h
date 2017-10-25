#include <list>
using namespace std;

class Process {
public:
    int pid;
    int AT;
    int TC;
    int CB;
    int IO;

    int prev_start_time;
    int prev_duration; //prev_start_time + prev_duration = current_fire_time
    int cb;
    int io;
    int remaining_time; // time to finish all TC
    int total_io;
    int total_cb;
    int finish_time;
    int wait_time;

    int static_prio; //static priority
    int dynamic_prio; //dynamic priority
    bool firstcompteCB;

    Process(int pid, int AT, int TC, int CB, int IO) : pid(pid), AT(AT), TC(TC), CB(CB), IO(IO) {
        prev_start_time = AT;
        prev_duration = 0;
        cb = 0;
        io = 0;
        remaining_time = TC;
        total_io = 0;
        total_cb = 0;
        finish_time = 0;
        wait_time = 0;
        firstcompteCB = true;
    }
};

class scheduler {
public:
    list<Process *> runqueue;
    list<Process *> expired_queue;
    virtual void add_to_queue(Process * p);
    virtual Process* get_next_process();
};

class FCFS: public scheduler {

    void add_to_queue(Process *p);
    Process* get_next_process();
};

class LCFS: public scheduler {
    void add_to_queue(Process *p);
    Process* get_next_process();
};

class SJF: public scheduler {
    void add_to_queue(Process *p);
    Process* get_next_process();
};

class RR: public scheduler {
    void add_to_queue(Process *p);
    Process* get_next_process();
};

class PRIO: public scheduler {
    void add_to_queue(Process *p);
    Process* get_next_process();
};

