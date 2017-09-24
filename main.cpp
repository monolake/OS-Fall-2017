#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>
#include <map>
#include <algorithm>

using namespace std;

int last_valid_pos = 0;
map<string, pair<int, int>> local_module_list;
map<string, pair<int,int>> symbol_table;
vector<int> module_global_addr;
int module_num;  // index of module, 1, 2, 3
int module_addr; //address of module, numInst
int cur_linenum; //start from 1

void parseerror(int errcode, int linenum, int lineoffset) {
    static char* errstr[] = {
        "NUM_EXPECTED",
        "SYM_EXPECTED",
        "ADDR_EXPECTED", //A/E/I/R
        "SYM_TOO_LONG",
        "TOO_MANY_DEF_IN_MODULE",
        "TOO_MANY_USE_IN_MODULE",
        "TOO_MANY_INSTR",
    };
    printf("Parse Error line %d offset %d: %s\n", linenum, lineoffset, errstr[errcode]);
}

string typeCheck(string token) {

    if (regex_match(token, regex("[[:alpha:]][[:alnum:]]{0,15}"))) {
        if (token != "I" && token != "A" && token != "R" && token != "E") {
            // this is a symbol
            //cout << "we read a symbol" << endl;
            return "sym";
        }
        else {
            // this is an instruction
            //cout << "we read an instruction" << endl;
            return "inst";
        }
    }
    else if (regex_match(token, regex("[[:digit:]][[:digit:]]*"))) {
            // this is a number
            //cout << "we read a number" << endl;
            return "num";
    }

    return "";

}
void readToken(ifstream & fin, int & cur_offset, string & token, string & last_token, int & last_line_offset, int type) {

    string temp_line;
    if (fin >> token) {

        //if (fin.peek() >= 'a' && fin.peek() <= 'z' || fin.peek() >= 'A' && fin.peek() <= 'Z' || fin.peek() >= '0' && fin.peek() <= '9') {
        if (last_line_offset == -1) {
            cur_offset = fin.tellg() - token.size() + 1;
        }
        else {
            cur_offset = fin.tellg() - last_line_offset - token.size();
        }
        last_token.assign(token);

        if (type == 0) { // a sym
            if (typeCheck(token) == "num") {
                parseerror(1, cur_linenum, cur_offset);
                exit(1);
            }
        }
        else if (type == 1) { // a num
            if (typeCheck(token) == "sym") {
                parseerror(0, cur_linenum, cur_offset);
                exit(1);
            }
        }

        while (fin.peek() != EOF and fin.peek() == '\n') {
            last_valid_pos = fin.tellg() - last_line_offset;

            //cout << "find newline with last line offset " << fin.tellg() << endl;
            last_line_offset = fin.tellg(); //update last_line_offset

            if (fin.get() == '\n') {
                cur_linenum++;
            }
            else {
                if (fin.tellg() != -1) {
                    fin.seekg(-1, fin.cur);
                }
                break;
            }
        }

    }
    else {
        if (type == 0) {
            parseerror(1, cur_linenum - 1, last_valid_pos);
        }
        else if (type == 2) {
            parseerror(2, cur_linenum - 1, last_valid_pos);
        }
        exit(1);
    }

}

string readSym(ifstream & fin, int & cur_offset, string & last_token, int & last_line_offset, int sym_type) {

    string token;
    readToken(fin, cur_offset, token, last_token, last_line_offset, sym_type);
    return token;

}

int readInt(ifstream & fin, int & cur_offset, string & last_token, int & last_line_offset) {

    string token;
    int type = 1;
    readToken(fin, cur_offset, token, last_token, last_line_offset, type);
    int number;
    if (typeCheck(token) == "num") {

        stringstream convert(token);
        convert >> number;
        convert.clear();

    }
    return number;
}
void readDef(ifstream & fin,
             int & cur_offset,
             string & last_token,
             int & last_line_offset) {
    int sym_type = 0;
    string sym = readSym(fin, cur_offset, last_token, last_line_offset, sym_type);
    //cout << "symbol " << sym << endl;
    int val = readInt(fin, cur_offset, last_token, last_line_offset);

    map<string, pair<int,int>>::iterator it = local_module_list.find(sym);
    if (it == local_module_list.end())
        local_module_list[sym] = make_pair(module_num, val + module_addr);

}

void readDef2(ifstream & fin,
             int & cur_offset,
             string & last_token,
             int & last_line_offset) {
    int sym_type = 0;
    string sym = readSym(fin, cur_offset, last_token, last_line_offset, sym_type);
    int val = readInt(fin, cur_offset, last_token, last_line_offset);
}


void readDefList(ifstream & fin, int & cur_offset, string & last_token,
                 int & last_line_offset) {
    //cout << "readDefList is called " << endl;
    int numDefs = readInt(fin, cur_offset, last_token, last_line_offset);
    if (numDefs > 16) {
        parseerror(4, cur_linenum, cur_offset);
        exit(1);
    }
    //cout << "numdef is " << numDefs << endl;
    for (int i = 0; i < numDefs; i++) {
        readDef(fin, cur_offset, last_token, last_line_offset);
    }
    //cout << "done with def " << endl;
}

void readDefList2(ifstream & fin, int & cur_offset, string & last_token, int & last_line_offset) {
    //cout << "readDefList is called " << endl;
    int numDefs = readInt(fin, cur_offset, last_token, last_line_offset);
    //cout << "numdef is " << numDefs << endl;
    for (int i = 0; i < numDefs; i++) {
        readDef2(fin, cur_offset, last_token, last_line_offset);
    }
    //cout << "done with def " << endl;
}

void readUse(ifstream & fin, int & cur_offset, string & last_token, int & last_line_offset) {
    int sym_type = 0;
    string sym = readSym(fin, cur_offset, last_token, last_line_offset, sym_type);
    //cout << " used symbol " << sym << endl;
}

void readUse2(ifstream & fin, int & cur_offset, string & last_token,
              vector<pair<string, bool>> & uselist,
              int & last_line_offset) {
    int sym_type = 0;
    string sym = readSym(fin, cur_offset, last_token, last_line_offset, sym_type);
    uselist.push_back(make_pair(sym, false));
    //cout << " used symbol " << sym << endl;
}

void readUseList(ifstream & fin, int & cur_offset, string & last_token, int & last_line_offset) {
    int numUse = readInt(fin, cur_offset, last_token, last_line_offset);
    if (numUse > 16) {
        parseerror(5, cur_linenum, cur_offset);
        exit(1);
    }
    for (int i = 0; i < numUse; i++) {
        readUse(fin, cur_offset, last_token, last_line_offset);
    }
    //cout << "done with use " << endl;
}

void readUseList2(ifstream & fin, int & cur_offset, string & last_token, vector<pair<string, bool>> & uselist, int & last_line_offset) {
    int numUse = readInt(fin, cur_offset, last_token, last_line_offset);
    for (int i = 0; i < numUse; i++) {
        readUse2(fin, cur_offset, last_token, uselist, last_line_offset);
    }
}

void readInst(ifstream & fin, int & cur_offset, string & last_token, int & last_line_offset) {
    int sym_type = 2; // inst
    string sym = readSym(fin, cur_offset, last_token, last_line_offset, sym_type);
    int val = readInt(fin, cur_offset, last_token, last_line_offset);
}


void readInstList(ifstream & fin, int & cur_offset, string & last_token, int & last_line_offset, int & total_inst) {
    int numInst = readInt(fin, cur_offset, last_token, last_line_offset);
    total_inst += numInst;
    if (total_inst > 512) {
        parseerror(6, cur_linenum, cur_offset);
        exit(1);
    }
    for (int i = 0; i < numInst; i++) {
        readInst(fin, cur_offset, last_token, last_line_offset);
    }

    //rule 5

    for (map<string, pair<int, int>>::iterator it = local_module_list.begin(); it != local_module_list.end(); ++it) {
        if (it->second.second - module_addr > numInst) {
            cout << "Warning: Module " << it->second.first << ": " << it->first << " too big " << it->second.second << " max=(" << numInst-1 << ") assume zero relative" << endl;
            it->second.second = 0 + module_addr;
        }


        if (symbol_table.find(it->first) == symbol_table.end()) {
            symbol_table[it->first] = it->second;
            cout << it->first << " = " << it->second.second << endl;
        }
        else {
            // rule 2
            cout << " Error: This variable is multiple times defined; first value used" << endl;
        }
    }
    local_module_list.clear();
    module_addr += numInst;
}

void readInst2(ifstream & fin,
               int & cur_offset,
               string & last_token,
               int cur_num,
               vector<pair<string, bool>> & uselist,
               vector<int> & instruction,
               int numInst,
               int & last_line_offset) {
    int sym_type = 2;
    string sym = readSym(fin, cur_offset, last_token, last_line_offset, sym_type);
    int val = readInt(fin, cur_offset, last_token, last_line_offset);
    //cout << "inst " << sym << " = " << val << endl;
    int result;
    if (sym == "E") {
        int opcode = val / 1000;
        int oprand = val % 1000;
        // rule 6
        if (oprand >= uselist.size()) {
            cout << cur_num + module_addr << " : " << val << " Error: External address exceeds length of uselist; treated as immediate" << endl;
        }
        else {
            string variable = uselist[oprand].first;
            // rule 3
            map<string, pair<int, int>>::iterator it = symbol_table.find(variable);
            if (it == symbol_table.end()) {
                result = opcode * 1000 + 0;
                cout << cur_num + module_addr << " : " << result << " ";
                cout << "Error: " << variable << " is not defined; zero used" << endl;
            }
            else {
                result = opcode * 1000 + symbol_table[variable].second;
                cout << cur_num + module_addr << " : " << result << endl;
            }
            uselist[oprand].second = true;
        }
    }
    else if (sym == "R") {
        int opcode = val / 1000;
        int oprand = val % 1000;
        // rule 9
        if (oprand > numInst) {
            result = opcode * 1000 + module_addr;
            cout << cur_num + module_addr << " : " << result << " Error: Relative address exceeds module size; zero used" << endl;
        }
        else {
            result = opcode * 1000 + oprand + module_addr;
            cout << cur_num + module_addr << " : " << result << endl;
        }
    }
    else if (sym == "A") {
        int opcode = val / 1000;
        int oprand = val % 1000;
        // rule 8
        if (oprand > 512) {
            oprand = 0;
            result = opcode * 1000 + oprand;
            cout << cur_num + module_addr << " : " << result << " Error: Absolute address exceeds machine size; zero used" << endl;
        }
        else {
            result = opcode * 1000 + oprand;
            cout << cur_num + module_addr << " : " << result << endl;
        }

    }
    else {
        result = val;
        cout << cur_num + module_addr << " : " << result << endl;
    }
    instruction.push_back(result);


}

void readInstList2(ifstream & fin,
                   int & cur_offset,
                   string & last_token,
                   vector<pair<string, bool>> & uselist,
                   vector<int> & instruction,
                   int & last_line_offset) {


    int numInst = readInt(fin, cur_offset, last_token, last_line_offset);
    for (int i = 0; i < numInst; i++) {
        readInst2(fin, cur_offset, last_token, i, uselist, instruction, numInst, last_line_offset);
    }
    module_addr += numInst;

    // rule 7
    for (int i = 0; i < uselist.size(); i++) {
        if (uselist[i].second == false) {
            cout << "Warning: Module " << module_num << " : " << uselist[i].first << " appeared in the uselist but was not actually used" << endl;
        }
    }


}

void pass1(string filename) {

    ifstream fin(filename.c_str());
    cur_linenum = 1;
    int cur_offset = 1;
    int last_line_offset = -1;
    string last_token;
    module_num = 0;
    fin.seekg(0, fin.end);
    int length = fin.tellg();
    fin.seekg(0, fin.beg);
    //cout << "length of file is " << length << endl;
    module_addr = 0;
    module_global_addr.push_back(module_addr);
    cout << "Symbol Table" << endl;
    int total_inst = 0;
    while (!fin.eof()) {
        // parse the input if file is open
        module_num++;
        readDefList(fin, cur_offset, last_token, last_line_offset);
        readUseList(fin, cur_offset, last_token, last_line_offset);
        readInstList(fin, cur_offset, last_token, last_line_offset, total_inst);
        module_global_addr.push_back(module_addr);
        if (fin.tellg() == length)
            break;
    }
    fin.close();

}

void pass2(string filename) {

    ifstream fin(filename.c_str());
    cur_linenum = 1;
    int cur_offset = 1;
    int last_line_offset = -1;
    string last_token;
    module_num = 0;
    fin.seekg(0, fin.end);
    int length = fin.tellg();
    fin.seekg(0, fin.beg);
    //cout << "length of file is " << length << endl;
    module_addr = 0;

    vector<int> instruction;
    vector<string> all_uselist;
    cout << "Memory Map" << endl;
    while (!fin.eof()) {
        // parse the input if file is open
        module_num++;
        vector<pair<string, bool>> uselist;
        readDefList2(fin, cur_offset, last_token, last_line_offset);
        readUseList2(fin, cur_offset, last_token, uselist, last_line_offset);
        readInstList2(fin, cur_offset, last_token, uselist, instruction, last_line_offset);
        for (auto item : uselist)
            all_uselist.push_back(item.first);

        if (fin.tellg() == length)
            break;
    }
    fin.close();
    //cout << "data has been processed " << endl;

    // rule 4
    for (auto item : symbol_table) {
        vector<string>::iterator it = find(all_uselist.begin(), all_uselist.end(), item.first);
        if (it == all_uselist.end())
            cout << "Warning: Module " << item.second.first  << " : " << item.first << " was defined but never used" << endl;
    }

}

int main() {

    string filename = "input-18";

    pass1(filename);
    pass2(filename);
}
