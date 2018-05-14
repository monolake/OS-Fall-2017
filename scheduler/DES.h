#include "scheduler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#define DEBUG 0
typedef enum STATE {STATE_CREATED, STATE_READY, STATE_RUNNING, STATE_BLOCKED, STATE_FINISHED} process_state_t;
typedef enum TRANS {TRANS_TO_READY, TRANS_TO_RUN, TRANS_TO_BLOCK, TRANS_TO_PREEMPT} state_transition_t;

class Event {

public:
    int timestamp;
    Process* process;
    process_state_t oldstate;
    process_state_t newstate;
    state_transition_t transition;

    int computeCB(int rand_num) {
        int num;
        if (process->remaining_time < rand_num) {
            num = process->remaining_time;
        }
        else {
            num = rand_num;
        }
        return num;
    }

    int computeIO(int rand_num) {
        return rand_num;
    }
};

class DES {

private:

    vector<Process*> plist;
    list<Event*> event_queue;
    scheduler * sched;
    vector<int> randvals;
    int total_rand_num;
    int quantum;
    int ofs;
    char sched_type;
public:
    DES(char sched_type, int quantum);
    ~DES();
    Event* getEvent();
    void putEvent(Event* event);
    void createEvent(int firetime, Process * p, process_state_t cur_state, process_state_t next_state, state_transition_t transition);
    void simulate();
    void printSummary();
    void readInput(string filename);
    void readRandom(string filename);
    int get_next_event_time();


};

