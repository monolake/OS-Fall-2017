#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <list>
#include "bithacks.h"
#include <algorithm>
#include <climits>

using namespace std;

typedef struct fte_struct {
    int pid;
    int vpage;
}FTE;

typedef long long int PTE;

#define FILE_MAPPED 13
#define PAGEOUT 12
#define PRESENT 11
#define REFERENCED 10
#define MODIFIED 9
#define WRITE_PROTECTED 8
// 0 - 7 page offset

typedef struct vma {
    int start_vpage;
    int end_vpage;
    int write_protected;
    int file_mapped;
}VMA;

class process {
public:
    PTE page_table[64];
    vector<VMA> vmas;
    long int unmaps;
    long int maps;
    long int ins;
    long int outs;
    long int fins;
    long int fouts;
    long int zeros;
    long int segv;
    long int segprot;
    long int reads;
    long int writes;
};

class Pager;

class VMM {
public:
    friend class Pager;
    friend class FIFO;
    friend class SC;
    friend class RD;
    friend class NRU;
    friend class CLOCK;
    friend class AGING;
    VMM(string filename, char algo, string num_frame);
    ~VMM();
    void simulate(VMM & vmm, string output);
    void printSummary();
    void readInput(string filename);
    void readRandom(string filename);
    bool get_next_instruction(char& operation, int& vpage);
    void update_pte();
private:
    int num_frames;
    vector<FTE> frame_table;
    list<int> frame_free_list;
    vector<process*> process_list;
    char operation;
    int vpage;
    int current_pid;
    Pager * THE_PAGER;
    vector<long long int> randvals;
    int total_rand_num;
    int num_lines_proc;
    ifstream fin;

    long int ctx_switches;
    long int inst_count;
    int ofs;
    long int clock;
    vector<unsigned long int> counters;


};

class Pager {
public:
    list<int> page_in_mem;
    virtual int get_victim_frame(VMM & vmm){}
};

class FIFO : public Pager {
public:
    int get_victim_frame(VMM & vmm) {
        if (!page_in_mem.empty()) {
            int frame_index = page_in_mem.front();
            page_in_mem.pop_front();
            return frame_index;
        }
    }

};

class SC : public Pager {
public:
    int get_victim_frame(VMM & vmm) {
        while (!page_in_mem.empty()) {
            int frame_index = page_in_mem.front();
            page_in_mem.pop_front();
            FTE head_fte = vmm.frame_table[frame_index];
            process* p = vmm.process_list[head_fte.pid];
            if (B_IS_SET(p->page_table[head_fte.vpage], REFERENCED)) {
                B_UNSET(p->page_table[head_fte.vpage], REFERENCED);
                page_in_mem.push_back(frame_index);
            }
            else {
                return frame_index;
            }
        }
    }
};

class RD: public Pager {
public:
    int get_victim_frame(VMM & vmm) {
        if (vmm.ofs >= vmm.total_rand_num)
            vmm.ofs = 0;
        int rand_num = (vmm.randvals[vmm.ofs] % vmm.num_frames);
        vmm.ofs++;
        int frame_index;
        for (list<int>::iterator it = page_in_mem.begin(); it != page_in_mem.end(); ++it) {
                if (*it == rand_num){
                    frame_index = *it;
                    page_in_mem.erase(it);
                    break;
                }
        }
        return frame_index;
    }
};

class NRU : public Pager {
public:
    int get_victim_frame(VMM & vmm) {



        vector<int> nru_0;
        vector<int> nru_1;
        vector<int> nru_2;
        vector<int> nru_3;

        for (int i = 0; i < vmm.num_frames; i++) {
            int pid = vmm.frame_table[i].pid;
            int vpage = vmm.frame_table[i].vpage;

            int R = B_IS_SET(vmm.process_list[pid]->page_table[vpage], REFERENCED);
            int M = B_IS_SET(vmm.process_list[pid]->page_table[vpage], MODIFIED);
            if (R == 0 && M == 0)
                nru_0.push_back(i);
            else if (R == 0 && M == 1)
                nru_1.push_back(i);
            else if (R == 1 && M == 0)
                nru_2.push_back(i);
            else if (R == 1 && M == 1)
                nru_3.push_back(i);

        }
        int frame_index;
      // cout << "random number is " << vmm.randvals[vmm.ofs] << endl;
       if (vmm.ofs >= vmm.total_rand_num)
            vmm.ofs = 0;

        if (!nru_0.empty()) {
            frame_index = nru_0[vmm.randvals[vmm.ofs] % nru_0.size()];
//            cout << "ASTATUS:  | selected --> lowest_class=0: selidx=" << vmm.randvals[vmm.ofs] % nru_0.size() << \
//             " from ";
//             for (int i = 0; i < nru_0.size(); i++)
//                cout << nru_0[i] << " ";
//             cout << endl;
        }
        else if (!nru_1.empty()) {
            frame_index = nru_1[vmm.randvals[vmm.ofs] % nru_1.size()];
//            cout << "ASTATUS:  | selected --> lowest_class=1: selidx=" << vmm.randvals[vmm.ofs] % nru_1.size() << \
//             " from ";
//             for (int i = 0; i < nru_1.size(); i++)
//                cout << nru_1[i] << " ";
//             cout << endl;
        }
        else if (!nru_2.empty()) {
            frame_index = nru_2[vmm.randvals[vmm.ofs] % nru_2.size()];
//            cout << "ASTATUS:  | selected --> lowest_class=2: selidx=" << vmm.randvals[vmm.ofs] % nru_2.size() << \
//             " from ";
//             for (int i = 0; i < nru_2.size(); i++)
//                cout << nru_2[i] << " ";
//             cout << endl;
        }
        else if (!nru_3.empty()) {
            frame_index = nru_3[vmm.randvals[vmm.ofs] % nru_3.size()];
//            cout << "ASTATUS:  | selected --> lowest_class=3: selidx=" << vmm.randvals[vmm.ofs] % nru_3.size() << \
//             " from ";
//             for (int i = 0; i < nru_3.size(); i++)
//                cout << nru_3[i] << " ";
//             cout << endl;
        }
        vmm.ofs++;



        for (list<int>::iterator it = page_in_mem.begin(); it != page_in_mem.end(); ++it) {
                if (*it == frame_index){
                    page_in_mem.erase(it);
                    break;
                }
        }

     vmm.clock++;
        //cout << "paging index " << clock << " " << operation << " " << vpage << endl;
        if (vmm.clock == 10) {
            //for (list<int>::iterator it = page_in_mem.begin(); it != page_in_mem.end(); ++it) {

            for (int i = 0; i < vmm.num_frames; i++) {
                int pid = vmm.frame_table[i].pid;
                int vpage = vmm.frame_table[i].vpage;
                B_UNSET(vmm.process_list[pid]->page_table[vpage], REFERENCED);
            }
            vmm.clock = 0;
        }

        return frame_index;
    }
};

class CLOCK : public Pager {


    int get_victim_frame(VMM & vmm) {
        FTE fte;
        int frame_index;
        while (true) {
            fte = vmm.frame_table[vmm.clock];
            int pid = fte.pid;
            int vpage = fte.vpage;
            if (B_IS_SET(vmm.process_list[pid]->page_table[vpage], REFERENCED)) {
                B_UNSET(vmm.process_list[pid]->page_table[vpage], REFERENCED);
                vmm.clock = (vmm.clock + 1) % vmm.num_frames;
            }
            else {
                frame_index = vmm.clock;
                vmm.clock = (vmm.clock + 1) % vmm.num_frames;
                break;
            }
        }
        return frame_index;
    }
};

class AGING : public Pager {



    int get_victim_frame(VMM & vmm) {

        int frame_index;
        unsigned long int counters_select = ULONG_MAX;
        for (int i = 0; i < vmm.num_frames; i++) {
            int pid = vmm.frame_table[i].pid;
            int vpage = vmm.frame_table[i].vpage;

            unsigned int R = B_IS_SET(vmm.process_list[pid]->page_table[vpage], REFERENCED);
            //if (R == 1)
            //    cout << "page " << pid << ":" << vpage << " is referenced" << endl;
            vmm.counters[i] = vmm.counters[i] >> 1;
            //cout << vmm.counters[i] << endl;
            R = R << 31;
            vmm.counters[i] = vmm.counters[i] | R;
            //cout << vmm.counters[i] << endl;
            if (vmm.counters[i] < counters_select) {
                counters_select = vmm.counters[i];
                frame_index = i;
            }
            B_UNSET(vmm.process_list[pid]->page_table[vpage], REFERENCED);
        }
        //vmm.clock++;
        vmm.counters[frame_index] = 0;
        return frame_index;
    }
};






