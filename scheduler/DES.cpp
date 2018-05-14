#include "DES.h"
#include <stdio.h>
#include <algorithm>

Event* DES::getEvent() {
    Event* event = event_queue.front();
    event_queue.pop_front();
    return event;
}

void DES::putEvent(Event* event) {
	// determine where to insert this event in event queue
	//cout << "event queue empty? " << event_queue.empty() << endl;
	//if (event_queue.empty() || event->timestamp >= event_queue.back()->timestamp)
       // event_queue.push_back(event);
    //else {
        list<Event*>::iterator it = event_queue.begin();

        for (; it != event_queue.end(); ++it) {
            if (event->timestamp >= (*it)->timestamp)
                continue;
            else {
                break;
            }
        }
        event_queue.insert(it, event);
//        cout << "current events ... " << endl;
//        for (list<Event*>::iterator it = event_queue.begin(); it != event_queue.end(); ++it) {
//            cout << "event pid " << (*it)->process->pid << " timestamp " << (*it)->timestamp << endl;
//        }
    //}
}

void DES::createEvent(int firetime, Process * p, process_state_t cur_state, process_state_t next_state, state_transition_t transition) {

    Event * event = new Event();
    event->timestamp = firetime;
    event->process = p;
    event->oldstate = cur_state;
    event->newstate = next_state;
    event->transition = transition;
    putEvent(event);
}

string intToEnum (process_state_t t) {
    switch (t) {
case 0:
    return "CREATED";
case 1:
    return "READY";
case 2:
    return "RUNNG";
case 3:
    return "BLOCK";
case 4:
    return "FINISHED";
    }
}

void DES::simulate() {


    int current_time;
    bool CALL_SCHEDULER = false;
    bool cpu_run = 0;
    Process * cur_running_p = nullptr;

    vector<pair<double, double>> io_time;
    while (!event_queue.empty()) {
        Event* event = getEvent();
        //cout << "time " << event->timestamp << " event->pid " << event->process->pid << " last state " << event->oldstate \
        << " new state " << event->newstate << endl;
        current_time = event->timestamp;
        switch(event->transition) {
            case TRANS_TO_READY: {

//                cout << event->timestamp << " " << event->process->pid << " " << event->process->prev_duration << ": " << \
//                intToEnum(event->oldstate) << "->" << intToEnum(event->newstate) << endl;
                if (event->oldstate == STATE_BLOCKED) {
                    io_time.push_back(make_pair(event->timestamp - event->process->prev_duration, event->timestamp));
                }

                CALL_SCHEDULER = true;

                event->process->prev_start_time = event->timestamp;
                event->process->prev_duration = 0;

                if (event->oldstate == STATE_CREATED) {
                    //if (sched->runqueue.front())
                    //    cout << "head of queue " << sched->runqueue.front()->pid << endl;
                    sched->add_to_queue(event->process);

                }
                else if (event->oldstate == STATE_BLOCKED) {
                    sched->add_to_queue(event->process);
                }
                else {
                    // event->oldstate == running, event->newstate == ready, come from preemption
                }
                break;
            }
            case TRANS_TO_RUN: {

                cur_running_p->wait_time += (current_time - cur_running_p->prev_start_time);
                if (quantum == 0) {
                    //event->process->prev_start_time;
                    //event->process->prev_duration;
                    // FCFS
                    if (DEBUG == 1) {
                        cout << "randvals[i] : " << randvals[ofs] << endl;
                        cout << "CB : " << event->process->CB << endl;
                    }
                    if (ofs >= total_rand_num)
                        ofs = 0;
                    int rand_num = 1 + (randvals[ofs] % event->process->CB);
                    ofs++;

                    event->process->cb = event->computeCB(rand_num); // assign cpu burst time
                    event->process->total_cb += event->process->cb;
//                    cout << event->timestamp << " " << event->process->pid << " " << event->process->prev_duration << ": " <<\
//                    intToEnum(event->oldstate) << "->" << intToEnum(event->newstate) << " cb=" << event->process->cb << " rem=" \
//                    << event->process->remaining_time << " prio=" << event->process->dynamic_prio << endl;
                    event->process->remaining_time -= event->process->cb; //remaining time after cpu burst

                    event->process->prev_start_time = event->timestamp;
                    event->process->prev_duration = event->process->cb;
                    int fire_time = event->process->prev_start_time + event->process->prev_duration;
                    createEvent(fire_time, event->process, STATE_RUNNING, STATE_BLOCKED, TRANS_TO_BLOCK);

                }
                else {
                    // RR
                    if (event->process->firstcompteCB) {
                        if (DEBUG == 1) {
                            cout << "randvals[i] : " << randvals[ofs] << endl;
                            cout << "CB : " << event->process->CB << endl;
                        }
                        if (ofs >= total_rand_num)
                            ofs = 0;
                        int rand_num = 1 + (randvals[ofs] % event->process->CB);
                        ofs++;

                        event->process->cb = event->computeCB(rand_num); // assign cpu burst time
                        //cout << event->process->cb << endl;
                        event->process->total_cb += event->process->cb;
                        //event->process->dynamic_prio = event->process->static_prio - 1;
                    }

                    if (quantum > event->process->cb) {
//                        cout << current_time << " " << event->process->pid << " " << event->process->prev_duration << ": " <<\
//                        intToEnum(event->oldstate) << "->" << intToEnum(event->newstate) << " cb=" << event->process->cb << " rem=" \
//                        << event->process->remaining_time << " prio=" << event->process->dynamic_prio << endl;
                        event->process->prev_duration = event->process->cb;
                        event->process->remaining_time -= event->process->cb; //remaining time after cpu burst
                        event->process->cb = 0;
                        event->process->firstcompteCB = true;

                        event->process->prev_start_time = event->timestamp;
                        int fire_time = event->process->prev_start_time + event->process->prev_duration;

                        createEvent(fire_time, event->process, STATE_RUNNING, STATE_BLOCKED, TRANS_TO_BLOCK);

                    }
                    else {
                        event->process->firstcompteCB = false;
                        // quantum smaller than process->cb, preempted
//                        cout << current_time << " " << event->process->pid << " " << event->process->prev_duration << ": " <<\
//                        intToEnum(event->oldstate) << "->" << intToEnum(event->newstate) << " cb=" << event->process->cb << " rem=" \
//                        << event->process->remaining_time << " prio=" << event->process->dynamic_prio << endl;
                        event->process->prev_duration = quantum;
                        event->process->remaining_time -= quantum; //remaining time after cpu burst
                        event->process->cb -= quantum;


                        event->process->prev_start_time = event->timestamp;
                        int fire_time = event->process->prev_start_time + event->process->prev_duration;
                        //cout << "process cb : " << event->process->cb << endl;

                        if (event->process->cb > 0) {
                            createEvent(fire_time, event->process, STATE_RUNNING, STATE_READY, TRANS_TO_PREEMPT);

                        }
                        else {
                            createEvent(fire_time, event->process, STATE_RUNNING, STATE_BLOCKED, TRANS_TO_BLOCK);
                            event->process->firstcompteCB = true;
                        }

                    }
                }
                break;
            }
            case TRANS_TO_BLOCK: {
                //now blocked, create event for ready, when it is ready
                cur_running_p = nullptr;
                //cout << "process " << event->process->pid << " remaining time " << event->process->remaining_time << endl;
                if (event->process->remaining_time == 0) {
                    //cout << "process " << event->process->pid << " finished!!" << endl;
                    //cout << "total io burst " << event->process->total_io << endl;
                    //cout << "total cb burst " << event->process->total_cb << endl;

                    //cout << current_time << " " << event->process->pid << " " << event->process->prev_duration << ": Done" << endl;
                    event->process->finish_time = current_time;

                    //cout << "remaining time of p0 " << plist[0]->remaining_time << endl;
                    //cout << "remaining time of p1 " << plist[1]->remaining_time << endl;
                    //cout << sched->runqueue.front()->pid << endl;
                    //cur_running_p = nullptr;
                    CALL_SCHEDULER = true;
                    break;
                }
                //create io burst for blocked event
                if (DEBUG == 1) {
                    cout << "randvals[i] : " << randvals[ofs] << endl;
                    cout << "IO : " << event->process->IO << endl;
                }

                if (ofs >= total_rand_num)
                    ofs = 0;
                int rand_num = 1 + (randvals[ofs] % event->process->IO);
                ofs++;

                event->process->io = event->computeIO(rand_num); // assign io burst time
                //cout << "IO burst: " << event->process->io << endl;
                event->process->total_io += event->process->io;

                //cout << "total IO burst: " << event->process->total_io << endl;
//                cout << current_time << " " << event->process->pid << " " << event->process->prev_duration << ": " <<\
//                intToEnum(event->oldstate) << "->" << intToEnum(event->newstate) << " ib=" << event->process->io << " rem=" \
//                << event->process->remaining_time << endl;
                CALL_SCHEDULER = true; // maybe for preemption

                event->process->prev_start_time = event->timestamp;
                event->process->prev_duration = event->process->io;
                //io_time.push_back(make_pair(event->timestamp, current_time+event->process->total_io));
                int fire_time = event->process->prev_start_time + event->process->prev_duration;
                if (sched_type == 'P' || sched_type == 'R')
                    event->process->dynamic_prio = event->process->static_prio - 1;
                createEvent(fire_time, event->process, STATE_BLOCKED, STATE_READY, TRANS_TO_READY);
                break;
            }
            case TRANS_TO_PREEMPT: {
                //now preempt, no event is generated
                cur_running_p = nullptr;
                CALL_SCHEDULER = true;
//                cout << current_time << " " << event->process->pid << " " << event->process->prev_duration << ": " <<\
//                intToEnum(event->oldstate) << "->" << intToEnum(event->newstate) << " cb=" << event->process->cb << " rem=" \
//                << event->process->remaining_time << " prio=" << event->process->dynamic_prio << endl;
                // time spent in preempt
                event->process->prev_start_time = event->timestamp;
                event->process->prev_duration = 0;
                if (sched_type == 'P' || sched_type == 'R') {
                    event->process->dynamic_prio -= 1;
                    sched->add_to_queue(event->process);
                }
                break;
            }
        }
        delete event;
        if (CALL_SCHEDULER) {
            if (get_next_event_time() == current_time)
                continue;
            //cout << "scheduler call" << endl;
            CALL_SCHEDULER = false;
            if(cur_running_p == nullptr) {
                cur_running_p = sched->get_next_process();

                if (cur_running_p == nullptr) {
                    //cout << "no more process available in runqueue" << endl;
                    continue;
                }
                else {
                    //cout << "cur_running_p " << cur_running_p->pid << endl;
                    //cout << "cur_running_process : " << cur_running_p->pid << endl;
                    //transit from ready to run

                    cur_running_p->prev_duration = current_time - cur_running_p->prev_start_time;
                    int fire_time = cur_running_p->prev_start_time + cur_running_p->prev_duration;

                    createEvent(fire_time, cur_running_p, STATE_READY, STATE_RUNNING, TRANS_TO_RUN);

                }
            }
            //else
            //    cout << "cur_running_p " << cur_running_p->pid << endl;
        }
    }

//    double a, b;
//    a = 1.0/3.0;
//    b = 2.0/3.0;
//    printf("%.21f %.21f\n", a, b);
//    printf("%.31f %.31f\n", a, b);
    int last_finish = 0;
    double total_wait = 0;
    double cpu_util = 0;
    double io_util = 0;
    double avg_turnaround = 0;
    double avg_wait = 0;
    double throughput = 0;

    if (this->sched_type == 'F') {
        cout << "FCFS" << endl;
    }
    else if (this->sched_type == 'L')
        cout << "LCFS" << endl;
    else if (this->sched_type == 'S')
        cout << "SJF" << endl;
    else if (this->sched_type == 'R')
        cout << "RR " << this->quantum << endl;
    else
        cout << "PRIO " << this->quantum << endl;
    for (vector<Process *>::iterator it = plist.begin(); it != plist.end(); ++it) {
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",
               (*it)->pid,
               (*it)->AT,
               (*it)->TC,
               (*it)->CB,
               (*it)->IO,
               (*it)->static_prio,
               (*it)->finish_time,
               (*it)->finish_time - (*it)->AT,
               (*it)->total_io,
               (*it)->wait_time);
        last_finish = max(last_finish, (*it)->finish_time);
        total_wait += (*it)->wait_time;
        cpu_util += (*it)->total_cb;
        //io_util += (*it)->total_io;
        avg_turnaround += (*it)->finish_time - (*it)->AT;
    }
    //io_util = (last_finish - 1) - cpu_util;
    cpu_util = cpu_util / last_finish * 100;
    //io_util = io_util / last_finish * 100;
    avg_turnaround = avg_turnaround / plist.size();
    avg_wait = total_wait / plist.size();
    throughput = 100.0 * (double) plist.size() / last_finish;

    // compute io_util

//    for (int i = 0; i < io_time.size(); i++) {
//        cout << io_time[i].first << " " << io_time[i].second << endl;
//    }
    sort(io_time.begin(), io_time.end());
//    for (int i = 0; i < io_time.size(); i++) {
//        cout << io_time[i].first << " " << io_time[i].second << endl;
//    }
    vector<pair<double, double>> result;
    result.push_back(io_time[0]);
    for (int i = 1; i < io_time.size(); i++) {
        if (result.back().second >= io_time[i].first) {
            result.back().second = max(result.back().second, io_time[i].second);
        }
        else
            result.push_back(io_time[i]);
    }

    for (int i = 0; i < result.size(); i++) {
        //cout << result[i].first << " " << result[i].second << endl;
        io_util += result[i].second - result[i].first;
    }
    io_util = io_util / last_finish * 100;
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",
            last_finish,
            cpu_util,
            io_util,
            avg_turnaround,
            avg_wait,
            throughput);

}

void DES::readInput(string filename) {
    ifstream fin;
    fin.open(filename.c_str());
    string line;
    int pid = 0;
    int AT, TC, CB, IO;
    while (getline(fin, line)) {
        istringstream token(line);
        token >> AT >> TC >> CB >> IO;
        plist.push_back(new Process(pid, AT, TC, CB, IO));
        //cout << "readinput " << AT << endl;
        createEvent(AT, plist[pid], STATE_CREATED, STATE_READY, TRANS_TO_READY);
        pid++;
    }
    //quantum = 2;
    fin.close();
}

void DES::readRandom(string filename) {
    ifstream fin;
    fin.open(filename.c_str());
    string line;
    getline(fin, line);
    istringstream token(line);
    token >> total_rand_num;
    int num;
    while(getline(fin, line)) {
        istringstream token(line);
        token >> num;
        randvals.push_back(num);
    }
    fin.close();

    ofs = 0;
    if (ofs >= total_rand_num)
        ofs = 0;
    for (vector<Process *>::iterator it = plist.begin(); it != plist.end(); ++it) {
        (*it)->static_prio = 1 + (randvals[ofs++] % 4);
        (*it)->dynamic_prio = (*it)->static_prio - 1;
    }

}

DES::~DES() {
    for (vector<Process*>::iterator it = plist.begin(); it != plist.end(); ++it) {
        delete *it;
    }
}

int DES::get_next_event_time() {
    Event * next = event_queue.front();
    if (next == nullptr)
        return -1000;
    else
        return next->timestamp;
}

DES::DES(char sched_type, int quantum) {

    if (sched_type == 'F') {
        this->sched = new FCFS();
        this->quantum = 0;
        this->sched_type = sched_type;
    }
    else if (sched_type == 'L') {
        this->sched = new LCFS();
        this->quantum = 0;
        this->sched_type = sched_type;
    }
    else if (sched_type == 'S') {
        this->sched = new SJF();
        this->quantum = 0;
        this->sched_type = sched_type;
    }
    else if (sched_type == 'R') {
        this->sched = new RR();
        this->quantum = quantum;
        this->sched_type = sched_type;
    }
    else {
        this->sched = new PRIO();
        this->quantum = quantum;
        this->sched_type = sched_type;
        //cout << "sched " << sched << " quantum " << quantum << endl;
    }
}

