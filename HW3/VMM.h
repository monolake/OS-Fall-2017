#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <list>
#include "bithacks.h"
#include <algorithm>

using namespace std;

typedef struct fte_struct {
    int pid;
    int vpage;
}FTE;

class Pager {
public:
    list<int> page_in_mem;
    virtual int get_victim_frame(){}
};

class FIFO : public Pager {
public:
    int get_victim_frame() {
        if (!page_in_mem.empty()) {
            int frame_index = page_in_mem.front();
            page_in_mem.pop_front();
            return frame_index;
        }
    }
};
//typedef struct struct_pte {
//    bool notpresent;
//    bool write_protect;
//    bool modified;
//    bool referenced;
//    bool paged_out;
//    bool filemapped;
//
//}PTE;
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



class VMM {
public:
    VMM(string filename);
    ~VMM();
    void simulate();
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
    vector<int> randvals;
    int total_rand_num;
    int num_lines_proc;
    ifstream fin;

    long int ctx_switches;
    long int inst_count;

};
