#include "IOsched.h"
IOsched::IOsched(char alg) {
    this->alg = alg;
    if (this->alg == 'i')
        sched = new FIFO();
    else if (this->alg == 'j')
        sched = new SSTF();
    else if (this->alg == 's')
        sched = new LOOK();
    else if (this->alg == 'c')
        sched = new CLOOK();
    else if (this->alg == 'f')
        sched = new FLOOK();

    maxtime = 0;
    current_time = 0;
    current_track = 0;
    req_track = 0;
    previous_track = 0;
    total_time = 0;
    total_movement = 0;
    avg_turnaround = 0;
    avg_waittime = 0;
    max_waittime = 0;
    num_io = 0;
}

void IOsched::simulate(IOsched & ios) {

    //cout << "size of io " << num_io << endl;
    for (int i = 0; i < io_generator.size(); i++) {
        IO_OPS * current_req = &(io_generator[i]);
        //printf("%5d: %5d %5d %5d\n", current_req->ops_num, current_req->arrive_time, current_req->start_time, current_req->finish_time);
    }
    int sim_finished = 0;
    int ops_added = 0;
    while (true) {
        if (sim_finished >= num_io)
            break;
        //if (sched->io_queue.empty() && io_generator.empty())
         //   break;
        //cout << "time " << current_time << " size of io " << sched->io_queue.size() << endl;

        if (sched->checkIOArrive(ios, ops_added)) {
           // cout << current_time << ": " << io_generator[ops_added].ops_num << " add " << io_generator[ops_added].track << endl;
            IO_OPS * ptr = &(io_generator[ops_added]);
            if (alg == 'f') {
                if(sched->checkIOComplete(ios) == true) {
                    //nothing running
                    sched->io_first_queue.push_back(ptr);
                    ops_added++;
                }
                else {
                    //something from first queue is running
                    sched->io_second_queue.push_back(ptr);
                    ops_added++;
                }
            }
            else {
                sched->io_queue.push_back(ptr);
                ops_added++;
                //io_generator.pop_front();
            }
        }
        // there is nothing running now, pick a request
        if (sched->checkIOComplete(ios) == true) {
            if (alg == 'f') {
                if (sched->io_first_queue.empty() && sched->io_second_queue.empty()) {
                    current_time++;
                    continue;
                }
            }
            else {
                if (sched->io_queue.empty()) {
                    current_time++;
                    continue;
                }
            }
            sched->io_complete = false;
            current_req = sched->selectReq(ios);
            //cout << current_time << ": " << current_req->ops_num  << " issue " << current_req->track << " " << current_track << endl;
            int arrive_time = current_req->arrive_time;
            req_track = current_req->track;
            int wait_time = current_time - arrive_time;
            int start_time = current_time;
            //cout << "arrive_time " << arrive_time << " start_time " << start_time << endl;
            //cout << "current_track " << current_track << " req_track " << req_track << endl;
            total_movement += abs(current_track - req_track);

            int finish_time = current_time + abs(current_track - req_track);
            avg_turnaround += (finish_time - arrive_time);
            avg_waittime += wait_time;
            max_waittime = max(max_waittime, wait_time);
            current_req->start_time = start_time;
            current_req->finish_time = finish_time;
            //
            previous_track = current_track;
            current_track = req_track;
            //cout << "current_track " << current_track << endl;
            //cout << "finish time " << finish_time << endl;
        }
        else {
            // something is running
            if (current_req->finish_time == current_time) {
                // io complete
                sched->io_complete = true;

                //cout << current_time << ": " << current_time << endl;
                //io_printer.push_back(current_req);
                sim_finished++;
                //sched->io_queue.pop_front();
            }
            else {
                current_time++;
            }
        }

    }

    for (int i = 0; i < io_generator.size(); i++) {
        IO_OPS current_req = io_generator[i];
        printf("%5d: %5d %5d %5d\n", current_req.ops_num, current_req.arrive_time, current_req.start_time, current_req.finish_time);
    }

    // print information
    total_time = current_time;
    avg_turnaround /= num_io;
    avg_waittime /= num_io;
    printf("SUM: %d %d %.2lf %.2lf %d\n", \
    total_time, total_movement, avg_turnaround, avg_waittime, max_waittime);
}

void IOsched::readInput(string filename) {
    ifstream fin;
    fin.open(filename.c_str());
    string line;
    getline(fin, line);
    getline(fin, line);
    while(getline(fin, line)) {
        int time, track;
        istringstream is(line);
        is >> time >> track;
        IO_OPS ops = {time, track, 0, 0, num_io};
        //IO_OPS * ops_p = ops;
        io_generator.push_back(ops);
        maxtime = max(maxtime, time);
        num_io++;
    }

    for (int i = 0; i < io_generator.size(); i++) {
    //    printf("%5d: %5d %5d %5d\n", io_generator[i].ops_num, io_generator[i].arrive_time, io_generator[i].start_time, io_generator[i].finish_time);
    }

    fin.close();
}

