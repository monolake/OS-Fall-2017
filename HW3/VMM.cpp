#include "VMM.h"

VMM::VMM(string filename) {
    fin.open(filename.c_str());
    num_frames = 32;
    for (int i = 0; i < num_frames; i++) {
        frame_free_list.push_back(i);
        FTE fte = {-1, -1};
        frame_table.push_back(fte);
    }
    THE_PAGER = new FIFO();
    ctx_switches = 0;
    inst_count = 0;

}

VMM::~VMM() {
    fin.close();
    delete THE_PAGER;
}

bool VMM::get_next_instruction(char& operation, int& vpage) {

    string line;
    getline(fin, line);
    cout << line << endl;
    if (line[0] == '#') {
        return false;
    }
    else {
        istringstream is(line);
        is >> operation >> vpage;
        return true;
    }

}


void VMM::simulate() {

    while (get_next_instruction(operation, vpage)) {
        //continue;
        //cout << operation << " " << vpage << endl;
        if (operation == 'c') {
            ctx_switches++;
            current_pid = vpage;
            cout << inst_count << ": ==> " << operation << " " << vpage << endl;
        }
        else {
            process* current_process = process_list[current_pid];
            PTE pte = current_process->page_table[vpage];
            //if pte->notpresent
            //cout << operation << " " << vpage << endl;
            if (!B_IS_SET(current_process->page_table[vpage], PRESENT)) {
                bool in_vma = false;
                for (int i = 0; i < current_process->vmas.size(); i++) {
                    //cout << "start " << current_process->vmas[i].start_vpage << endl;
                    //cout << "end " << current_process->vmas[i].end_vpage << endl;
                    if (vpage >= current_process->vmas[i].start_vpage && \
                        vpage <= current_process->vmas[i].end_vpage) {
                            in_vma = true;
                            //B_SET(current_process->page_table[vpage], WRITE_PROTECTED);
                            break;
                        }
                }
                if (in_vma == false) {
                    current_process->segv++;
                    //cout << "raise seg fault" << endl;
                    cout << inst_count << ": ==> " << operation << " " << vpage << endl;
                    cout << " SEGV" << endl;
                    inst_count++;
                    if (operation == 'r')
                        current_process->reads++;
                    else if (operation == 'w')
                        current_process->writes++;
                    continue;
                }
                if (!frame_free_list.empty()) {
                    int frame_index = frame_free_list.front();
                    frame_free_list.pop_front();
                    int write_protected = 0;
                    if (B_IS_SET(current_process->page_table[vpage], WRITE_PROTECTED))
                        write_protected = 1;
                    int file_mapped = 0;
                    if (B_IS_SET(current_process->page_table[vpage], FILE_MAPPED))
                        file_mapped = 1;
                    current_process->page_table[vpage] = current_process->page_table[vpage] & frame_index;
                    if (write_protected)
                        B_SET(current_process->page_table[vpage], WRITE_PROTECTED);
                    if (file_mapped)
                        B_SET(current_process->page_table[vpage], FILE_MAPPED);
                    //POPULATE THE PAGE WITH THE PROPER CONTENT
                    B_SET(current_process->page_table[vpage], PRESENT);
                    B_SET(current_process->page_table[vpage], REFERENCED);
                    frame_table[frame_index].pid = current_pid;
                    frame_table[frame_index].vpage = vpage;
                    THE_PAGER->page_in_mem.push_back(frame_index);
                    cout << inst_count << ": ==> " << operation << " " << vpage << endl;

                    if (B_IS_SET(current_process->page_table[vpage], FILE_MAPPED)) {
                        cout << " FIN" << endl;
                        current_process->fins++;
                    }
                    else {
                        if (!B_IS_SET(current_process->page_table[vpage], PAGEOUT)) {
                            cout << " ZERO" << endl;
                            current_process->zeros++;
                        }
                        else {
                            // page must be brought back from the swap sapce ("IN")
                            cout << " IN" << endl;
                            current_process->ins++;
                        }
                    }
                    cout << " MAP " << frame_index << endl;
                    current_process->maps++;
                }
                else {
                    //paging
                    cout << inst_count << ": ==> " << operation << " " << vpage << endl;

                    // UNMAP
                    int frame_index = THE_PAGER->get_victim_frame();
                    FTE unmapped_fte = frame_table[frame_index];
                    cout << " UNMAP " << unmapped_fte.pid << ":" << unmapped_fte.vpage << endl;
                    B_UNSET(process_list[unmapped_fte.pid]->page_table[unmapped_fte.vpage], PRESENT);
                    B_UNSET(process_list[unmapped_fte.pid]->page_table[unmapped_fte.vpage], REFERENCED);
                    process_list[unmapped_fte.pid]->unmaps++;

                    if(B_IS_SET(process_list[unmapped_fte.pid]->page_table[unmapped_fte.vpage], MODIFIED)) {
                        if (B_IS_SET(process_list[unmapped_fte.pid]->page_table[unmapped_fte.vpage], FILE_MAPPED)) {
                            cout << " FOUT " << endl;
                            process_list[unmapped_fte.pid]->fouts++;
                        }
                        else {
                            cout << " OUT " << endl;
                            process_list[unmapped_fte.pid]->outs++;
                        }
                        B_UNSET(process_list[unmapped_fte.pid]->page_table[unmapped_fte.vpage], MODIFIED);
                        B_SET(process_list[unmapped_fte.pid]->page_table[unmapped_fte.vpage], PAGEOUT);
                    }

                    //POPULATE THE PAGE WITH THE PROPER CONTENT
                    B_SET(current_process->page_table[vpage], PRESENT);
                    B_SET(current_process->page_table[vpage], REFERENCED);
                    frame_table[frame_index].pid = current_pid;
                    frame_table[frame_index].vpage = vpage;
                    THE_PAGER->page_in_mem.push_back(frame_index);

                    if (B_IS_SET(current_process->page_table[vpage], FILE_MAPPED)) {
                        cout << " FIN" << endl;
                        current_process->fins++;
                    }
                    else {
                        if (!B_IS_SET(current_process->page_table[vpage], PAGEOUT)) {
                            cout << " ZERO" << endl;
                            current_process->zeros++;
                        }
                        else {
                            // page must be brought back from the swap sapce ("IN")
                            cout << " IN" << endl;
                            current_process->ins++;
                        }
                    }
                    cout << " MAP " << frame_index << endl;
                    current_process->maps++;
                }
            }
            else {
                // page is present
                cout << inst_count << ": ==> " << operation << " " << vpage << endl;

            }
            if (operation == 'r') {
                current_process->reads++;
                B_SET(current_process->page_table[vpage], REFERENCED);
            }
            else if (operation == 'w') {
                current_process->writes++;
                B_SET(current_process->page_table[vpage], REFERENCED);
                cout << vpage << endl;
                if (B_IS_SET(current_process->page_table[vpage], WRITE_PROTECTED)) {
                    cout << " SEGPROT" << endl;
                    current_process->segprot++;

                   // break;
                }
                else {
                    B_SET(current_process->page_table[vpage], MODIFIED);
                    //cout << "seg protected ccccccccccccccccccccccccccccccccccccccccccccccccccccc " << endl;
                }

            }
        }
        inst_count++;
    }

    for (int i = 0; i < process_list.size(); i++) {
        process * proc = process_list[i];
        cout << "PT[" << i << "]: ";
        for (int j = 0; j < 64; j++) {

            if (B_IS_SET(proc->page_table[j], PRESENT)) {
                cout << j << ":";
                if (B_IS_SET(proc->page_table[j], REFERENCED))
                    cout<<"R";
                else
                    cout<<"-";
                if(B_IS_SET(proc->page_table[j],MODIFIED))
                    cout<<"M";
                else
                    cout<<"-";
                if(B_IS_SET(proc->page_table[j],PAGEOUT))
                    cout<<"S ";
                else
                    cout<<"- ";
            }
            else {
                if(B_IS_SET(proc->page_table[j],PAGEOUT))
                    cout<<"# ";
                else
                    cout<<"* ";
            }

        }
        cout<<endl;
    }
    cout << "FT: ";
    for (int i = 0; i < frame_table.size(); i++) {
            if (find(THE_PAGER->page_in_mem.begin(), THE_PAGER->page_in_mem.end(), i) != THE_PAGER->page_in_mem.end())
                cout << frame_table[i].pid << ":" << frame_table[i].vpage << " ";
            else
                cout << "* ";
    }
    cout << endl;
    for (int i = 0; i < process_list.size(); i++) {
        process * proc = process_list[i];
        printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n", \
               i, proc->unmaps, proc->maps, proc->ins, proc->outs, proc->fins, proc->fouts, proc->zeros, \
               proc->segv, proc->segprot);
    }
    long long int totalcost = 0;
    for (int i = 0; i < process_list.size(); i++) {
        process * proc = process_list[i];
        //printf("TOTALCOST %lu %lu %llu\n", ctx_switches, inst_count, totalcost);
        totalcost += 400 * (proc->maps + proc->unmaps);
        //printf("TOTALCOST %lu %lu %llu\n", ctx_switches, inst_count, totalcost);
        totalcost += 3000 * (proc->ins + proc->outs);
        //printf("TOTALCOST %lu %lu %llu\n", ctx_switches, inst_count, totalcost);
        totalcost += 2500 * (proc->fins + proc->fouts);
        //printf("TOTALCOST %lu %lu %llu\n", ctx_switches, inst_count, totalcost);
        totalcost += 150 * proc->zeros;
        //printf("TOTALCOST %lu %lu %llu\n", ctx_switches, inst_count, totalcost);
        totalcost += 240 * proc->segv;
        //printf("TOTALCOST %lu %lu %llu\n", ctx_switches, inst_count, totalcost);
        totalcost += 300 * proc->segprot;
        //printf("TOTALCOST %lu %lu %llu\n", ctx_switches, inst_count, totalcost);
        totalcost += 1 * (proc->reads + proc->writes);
        //printf("TOTALCOST %lu %lu %llu\n", ctx_switches, inst_count, totalcost);
    }
    totalcost += 121 * ctx_switches;
    printf("TOTALCOST %lu %lu %llu\n", ctx_switches, inst_count, totalcost);
}


void VMM::readInput(string filename) {

    string line;
    num_lines_proc = 0;
    for (int i = 0; i < 3; i++) {
        getline(fin, line);
        cout << line << endl;
        num_lines_proc++;
    }
    getline(fin, line);
    num_lines_proc++;
    istringstream is(line);
    int num_proc;
    is >> num_proc;
    cout << "number of process " << num_proc << endl;
    for (int i = 0; i < num_proc; i++) {
        getline(fin, line);
        getline(fin, line);
        getline(fin, line);
        num_lines_proc += 3;
        int vmas;
        istringstream is(line);
        is >> vmas;
        cout << "number of vma " << vmas << endl;
        process * p = new process();
        for (int j = 0; j < vmas; j++) {
            getline(fin, line);
            cout << line << endl;
            num_lines_proc++;
            int start_vpage, end_vpage;
            int write_protected, file_mapped;
            istringstream is(line);
            is >> start_vpage >> end_vpage >> write_protected >> file_mapped;
            VMA area = {start_vpage, end_vpage, write_protected, file_mapped};
            p->vmas.push_back(area);
        }
        for (int i = 0; i < 64; i++) {
            p->page_table[i] = 0;
            for (int j = 0; j < p->vmas.size(); j++) {
                if (i >= p->vmas[j].start_vpage && i <= p->vmas[j].end_vpage) {
                    if (p->vmas[j].write_protected == 1)
                        B_SET(p->page_table[i], WRITE_PROTECTED);
                    if (p->vmas[j].file_mapped == 1)
                        B_SET(p->page_table[i], FILE_MAPPED);
                }
            }
            if (B_IS_SET(p->page_table[i], FILE_MAPPED))
                cout << "thisis    right " << i << endl;
        }
        p->unmaps = 0;
        p->maps = 0;
        p->ins = 0;
        p->outs = 0;
        p->fins = 0;
        p->fouts = 0;
        p->zeros = 0;
        p->segv = 0;
        p->segprot = 0;
        p->reads = 0;
        p->writes = 0;
        process_list.push_back(p);
    }
    getline(fin, line);

}

void VMM::readRandom(string filename) {

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


}
