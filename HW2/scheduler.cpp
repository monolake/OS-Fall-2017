#include "scheduler.h"
#include <iostream>
#include <climits>
using namespace std;

void scheduler::add_to_queue(Process *p) {}
Process* scheduler::get_next_process() {}

void FCFS::add_to_queue(Process *p) {
    runqueue.push_back(p);
}

Process* FCFS::get_next_process() {
    if (runqueue.empty()) {
        //cout << "nothing in runqueue" << endl;
        return nullptr;
    }
    Process* p = runqueue.front();
    runqueue.pop_front();
    return p;
}

void LCFS::add_to_queue(Process *p) {
    runqueue.push_back(p);
    //cout << p->pid << " pushed into queue" << endl;
}

Process* LCFS::get_next_process() {
    if (runqueue.empty()) {
        //cout << "nothing in runqueue" << endl;
        return nullptr;
    }
    Process* p = runqueue.back();
    runqueue.pop_back();
    //cout << p->pid << " poped outof queue" << endl;
    return p;
}

void SJF::add_to_queue(Process *p) {
    runqueue.push_back(p);
}

Process* SJF::get_next_process() {
    if (runqueue.empty()) {
        //cout << "nothing in runqueue" << endl;
        return nullptr;
    }

    // GET PROCESS WITH MIN REMAINING TIME
    Process* p;
    int min_remain = INT_MAX;
    list<Process *>::iterator it_remove;
    for (list<Process *>::iterator it = runqueue.begin(); it != runqueue.end(); ++it) {
        if (min_remain > (*it)->remaining_time) {
            min_remain = (*it)->remaining_time;
            p = (*it);
            it_remove = it;
        }
    }
    runqueue.erase(it_remove);
    return p;
}

void RR::add_to_queue(Process *p) {
    runqueue.push_back(p);
}

Process* RR::get_next_process() {
    if (runqueue.empty()) {
        //cout << "nothing in runqueue" << endl;
        return nullptr;
    }

    Process* p = runqueue.front();
    runqueue.pop_front();
    return p;
}

void PRIO::add_to_queue(Process *p) {

    if (p->dynamic_prio == -1) {
        p->dynamic_prio = p->static_prio - 1;
        expired_queue.push_back(p);
    }
    else {
        // block to ready
//        list<Process *>::iterator it;
//        for (it = runqueue.begin(); it != runqueue.end(); ++it) {
//            if ((*it)->dynamic_prio < p->dynamic_prio) {
//                break;
//            }
//        }
//        runqueue.insert(it, p);
        runqueue.push_back(p);
    }

}

Process* PRIO::get_next_process() {

    if (runqueue.empty() && expired_queue.empty()) {
        return nullptr;
    }
    else if (runqueue.empty()) {
        list<Process *>::iterator it;
        while (!expired_queue.empty()) {
            Process *p = expired_queue.front();
            expired_queue.pop_front();
//            for (it = runqueue.begin(); it != runqueue.end(); ++it) {
//                if ((*it)->dynamic_prio < p->dynamic_prio) {
//                    break;
//                }
//            }
//            runqueue.insert(it, p);
            runqueue.push_back(p);

        }
    }
    for (list<Process *>::iterator it = runqueue.begin(); it != runqueue.end(); ++it) {
        //cout << "runqueue " << (*it)->pid << " prio " << (*it)->dynamic_prio << endl;
    }
    for (list<Process *>::iterator it = expired_queue.begin(); it != expired_queue.end(); ++it) {
        //cout << "expired " << (*it)->pid << " prio " << (*it)->dynamic_prio << endl;
    }
    // GET PROCESS WITH HIGHEST PRIORITY
    Process* p;
    int max_prio = -1000; //INT_MIN;
    list<Process *>::iterator it_remove;
    for (list<Process *>::iterator it = runqueue.begin(); it != runqueue.end(); ++it) {
        if (max_prio < (*it)->dynamic_prio) {
            max_prio = (*it)->dynamic_prio;
            p = (*it);
            it_remove = it;
        }
    }
    runqueue.erase(it_remove);

    return p;
}
