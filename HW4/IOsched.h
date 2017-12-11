#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <utility>
#include <stdlib.h>
#include <climits>

// 0 - arrive time
// 1 - track
// 2 - start time
// 3 - finish time
// 4 - operation number

using namespace std;

#define flook 0
typedef struct operation {
    int arrive_time;
    int track;
    int start_time;
    int finish_time;
    int ops_num;
}IO_OPS;

class Scheduler;
class IOsched {
public:

    //friend class Scheduler;
    //friend class FIFO;

    void readInput(string filename);
    void simulate(IOsched & ios);
    IOsched(char alg);
    vector<IO_OPS> io_generator;
    Scheduler * sched;
    int maxtime;
    int current_time;
    int req_track;
    int current_track;
    int previous_track;
    int total_time;
    int total_movement;
    double avg_turnaround;
    double avg_waittime;
    int max_waittime;
    int num_io;
    IO_OPS * current_req;
    //list<IO_OPS *> io_printer;
    char alg;
};

class Scheduler {
public:
    list<IO_OPS *> io_queue;
    list<IO_OPS *> io_first_queue;
    list<IO_OPS *> io_second_queue;
    bool io_complete;

    Scheduler() {
        io_complete = true;
    }
    bool checkIOArrive(IOsched & ios, int ops_added) {
        if (!ios.io_generator.empty() && ios.current_time == ios.io_generator[ops_added].arrive_time) {
            return true;
        }
        else {
            return false;
        }
    }
    bool checkIOComplete(IOsched & ios) {
        if (io_complete)
            return true;
        else
            return false;
    }
    virtual IO_OPS *  selectReq(IOsched & ios) {
    }

};

class FIFO : public Scheduler {
public:
    IO_OPS *  selectReq(IOsched & ios) {
        IO_OPS * temp = io_queue.front();
        io_queue.pop_front();
        return temp;
    }
};

class SSTF : public Scheduler {
public:
    IO_OPS * selectReq(IOsched & ios) {
        int current_track = ios.current_track;
        int track_diff = INT_MAX;
        list<IO_OPS *>::iterator it;
        IO_OPS * temp;
        list<IO_OPS *>::iterator delete_it;
        for (it = io_queue.begin(); it != io_queue.end(); ++it) {
            if (abs((*it)->track - current_track) < track_diff) {
                track_diff = abs((*it)->track - current_track);
                temp = (*it);
                delete_it = it;
            }
        }
        io_queue.erase(delete_it);
        return temp;
    }
};

class LOOK : public Scheduler {
public:
    bool up;
    LOOK() {
        up = true;
    }

    IO_OPS * selectReq(IOsched & ios) {
        //int prev_diff = ios.current_track - ios.previous_track;

        int current_track = ios.current_track;
        list<IO_OPS *>::iterator it;
/*
        cout << "current track " << current_track << endl;
        for (it = io_queue.begin(); it != io_queue.end(); ++it) {
            cout << (*it)->track << " ";
        }
        cout << endl;
*/
        if (up == true) {
            int flag = 0;
            for (it = io_queue.begin(); it != io_queue.end(); ++it) {
                if ((*it)->track > current_track) {
                        flag = 1;
                        break;
                }
            }
            if (flag == 1) {
                IO_OPS * temp;
                list<IO_OPS *>::iterator delete_it;
                int track_diff = INT_MAX;
                for (it = io_queue.begin(); it != io_queue.end(); ++it) {
                    if ((*it)->track >= current_track && abs((*it)->track - current_track) < track_diff) {
                        track_diff = abs((*it)->track - current_track);
                        temp = (*it);
                        delete_it = it;
                    }
                }
                io_queue.erase(delete_it);
                return temp;
            }
            else {
                up = false; // change direction
                IO_OPS * temp;
                list<IO_OPS *>::iterator delete_it;
                int track_diff = INT_MAX;
                for (it = io_queue.begin(); it != io_queue.end(); ++it) {
                    if ((*it)->track <= current_track && abs((*it)->track - current_track) < track_diff) {
                        track_diff = abs((*it)->track - current_track);
                        temp = (*it);
                        delete_it = it;
                    }
                }
                io_queue.erase(delete_it);
                return temp;
            }
        }
        else {
            int flag = 0;
            for (it = io_queue.begin(); it != io_queue.end(); ++it) {
                if ((*it)->track < current_track) {
                        flag = 1;
                        break;
                }
            }
            if (flag == 1) {
                IO_OPS * temp;
                list<IO_OPS *>::iterator delete_it;
                int track_diff = INT_MAX;
                for (it = io_queue.begin(); it != io_queue.end(); ++it) {
                    if ((*it)->track <= current_track && abs((*it)->track - current_track) < track_diff) {
                        track_diff = abs((*it)->track - current_track);
                        temp = (*it);
                        delete_it = it;
                    }
                }
                io_queue.erase(delete_it);
                return temp;
            }
            else {
                up = true; // change direction
                list<IO_OPS *>::iterator it;
                IO_OPS * temp;
                list<IO_OPS *>::iterator delete_it;
                int track_diff = INT_MAX;
                for (it = io_queue.begin(); it != io_queue.end(); ++it) {
                    if ((*it)->track >= current_track && abs((*it)->track - current_track) < track_diff) {
                        track_diff = abs((*it)->track - current_track);
                        temp = (*it);
                        delete_it = it;
                    }
                }
                io_queue.erase(delete_it);
                return temp;
            }

        }
    }

};

class CLOOK : public Scheduler {
public:
    //look if anything in up direction, if yes, pick it, if no, move current_track to smallest, then pick.
    IO_OPS * selectReq(IOsched & ios) {

        int current_track = ios.current_track;
        list<IO_OPS *>::iterator it;

        // cout << "current track " << current_track << endl;
        for (it = io_queue.begin(); it != io_queue.end(); ++it) {
            // cout << (*it)->track << " ";
        }
        // cout << endl;

        int flag = 0;
        for (it = io_queue.begin(); it != io_queue.end(); ++it) {
            if ((*it)->track > current_track) {
                    flag = 1;
                    break;
            }
        }
        if (flag == 1) {
            IO_OPS * temp;
            list<IO_OPS *>::iterator delete_it;
            int track_diff = INT_MAX;
            for (it = io_queue.begin(); it != io_queue.end(); ++it) {
                if ((*it)->track >= current_track && abs((*it)->track - current_track) < track_diff) {
                    track_diff = abs((*it)->track - current_track);
                    temp = (*it);
                    delete_it = it;
                }
            }
            io_queue.erase(delete_it);
            return temp;
        }
        else {
            // look for smallest track number
            int track_num = INT_MAX;
            IO_OPS * temp;
            for (it = io_queue.begin(); it != io_queue.end(); ++it) {
                if ((*it)->track < track_num) {
                    track_num = (*it)->track;
                    temp = (*it);
                }
            }
            // set current track to it, then look for another track to assign
            //ios.current_track = track_num;
            //current_track = ios.current_track;

            // you need to move the header to furthest point
            list<IO_OPS *>::iterator delete_it;
            int track_diff = INT_MIN;
            for (it = io_queue.begin(); it != io_queue.end(); ++it) {
                if ((*it)->track <= current_track && abs((*it)->track - current_track) > track_diff) {
                    track_diff = abs((*it)->track - current_track);
                    temp = (*it);
                    delete_it = it;
                }
            }
            io_queue.erase(delete_it);
            return temp;
        }

    }
};

class FLOOK : public Scheduler {
public:
    bool up;
    FLOOK() {
        up = true;
    }

    IO_OPS * selectReq(IOsched & ios) {
        //int prev_diff = ios.current_track - ios.previous_track;

        int current_track = ios.current_track;
        list<IO_OPS *>::iterator it;
/*
        cout << "current track " << current_track << endl;
        for (it = io_queue.begin(); it != io_queue.end(); ++it) {
            cout << (*it)->track << " ";
        }
        cout << endl;
*/
        if (io_first_queue.empty()) {
            while (!io_second_queue.empty()) {
                io_first_queue.push_back(io_second_queue.front());
                io_second_queue.pop_front();
            }
        }
        if (up == true) {
            int flag = 0;
            for (it = io_first_queue.begin(); it != io_first_queue.end(); ++it) {
                if ((*it)->track > current_track) {
                        flag = 1;
                        break;
                }
            }
            if (flag == 1) {
                IO_OPS * temp;
                list<IO_OPS *>::iterator delete_it;
                int track_diff = INT_MAX;
                for (it = io_first_queue.begin(); it != io_first_queue.end(); ++it) {
                    if ((*it)->track >= current_track && abs((*it)->track - current_track) < track_diff) {
                        track_diff = abs((*it)->track - current_track);
                        temp = (*it);
                        delete_it = it;
                    }
                }
                io_first_queue.erase(delete_it);
                return temp;
            }
            else {
                up = false; // change direction
                IO_OPS * temp;
                list<IO_OPS *>::iterator delete_it;
                int track_diff = INT_MAX;
                for (it = io_first_queue.begin(); it != io_first_queue.end(); ++it) {
                    if ((*it)->track <= current_track && abs((*it)->track - current_track) < track_diff) {
                        track_diff = abs((*it)->track - current_track);
                        temp = (*it);
                        delete_it = it;
                    }
                }
                io_first_queue.erase(delete_it);
                return temp;
            }
        }
        else {
            int flag = 0;
            for (it = io_first_queue.begin(); it != io_first_queue.end(); ++it) {
                if ((*it)->track < current_track) {
                        flag = 1;
                        break;
                }
            }
            if (flag == 1) {
                IO_OPS * temp;
                list<IO_OPS *>::iterator delete_it;
                int track_diff = INT_MAX;
                for (it = io_first_queue.begin(); it != io_first_queue.end(); ++it) {
                    if ((*it)->track <= current_track && abs((*it)->track - current_track) < track_diff) {
                        track_diff = abs((*it)->track - current_track);
                        temp = (*it);
                        delete_it = it;
                    }
                }
                io_first_queue.erase(delete_it);
                return temp;
            }
            else {
                up = true; // change direction
                list<IO_OPS *>::iterator it;
                IO_OPS * temp;
                list<IO_OPS *>::iterator delete_it;
                int track_diff = INT_MAX;
                for (it = io_first_queue.begin(); it != io_first_queue.end(); ++it) {
                    if ((*it)->track >= current_track && abs((*it)->track - current_track) < track_diff) {
                        track_diff = abs((*it)->track - current_track);
                        temp = (*it);
                        delete_it = it;
                    }
                }
                io_first_queue.erase(delete_it);
                return temp;
            }

        }
    }
};



