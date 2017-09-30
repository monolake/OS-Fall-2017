#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
//#include <regex>
#include <map>
#include <algorithm>

using namespace std;

int last_valid_pos = 0;
map<string, pair<int, int>> local_module_list; //rule 5
map<string, pair<int,int>> symbol_table;
map<string, bool> symbol_flag;
vector<int> module_global_addr;
int module_num;  // index of module, 1, 2, 3
int module_addr; //address of module, numInst
int cur_linenum; //start from 1
int cur_offset; //start from 0
ifstream fin;
int last_line_offset;//start from -1, first line

void parseerror(int errcode, int linenum, int lineoffset) {
    static char* errstr[] = {
        "NUM_EXPECTED",
        "SYM_EXPECTED",
        "ADDR_EXPECTED", //A/E/I/R
        "SYM_TOO_LONG",
        "TO_MANY_DEF_IN_MODULE",
        "TO_MANY_USE_IN_MODULE",
        "TO_MANY_INSTR",
    };
    printf("Parse Error line %d offset %d: %s\n", linenum, lineoffset, errstr[errcode]);
}

string typeCheck(string token) {

//    if (regex_match(token, regex("[[:alpha:]][[:alnum:]]{0,15}"))) {
//        if (token != "I" && token != "A" && token != "R" && token != "E") {
//            // this is a symbol
//            return "sym";
//        }
//        else {
//            return "inst";
//        }
//    }
//    else if (regex_match(token, regex("[[:digit:]][[:digit:]]*"))) {
//            // this is a number
//            return "num";
//    }

    if (token == "A" || token == "R" || token == "I" || token == "E")
        return "inst";
    else {
        for (int i = 0; i < token.size(); i++) {
            if (token[i] >= 'a' && token[i] <= 'z' || token[i] >= 'A' && token[i] <= 'Z') {
                return "sym";
            }
        }
    }


    return "num";

}

void readToken(string & token, int type, char & c) {

    token.clear();
    while (fin.peek() != EOF) {
        fin.get(c);

        if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9') {

            token += c;
            cur_offset++;
            //cout << "get c " << c << " at offset " << cur_offset << endl;
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


        }
        else if (c == '\n') {
            cur_offset++;
            //cout <<"get c newline" << " at offset " << cur_offset << endl;
            cur_linenum++;
            last_valid_pos = cur_offset;
            cur_offset = 0;
            if (token.length() != 0) {
                //cout << "token " << token << endl;
                return;
            }

        }
        else if (c ==' ' || c == '\t') {

            cur_offset++;
            //cout <<"get c space" << " at offset " << cur_offset << endl;
            if (token.length() != 0) {
                //cout << "token " << token << endl;
                return;
            }
        }
    }
    //cout << "type is " << type << endl;
    if (type == 0) { //sym
        parseerror(1, cur_linenum - 1, last_valid_pos);
    }
    else if (type == 2) { //inst
        parseerror(2, cur_linenum - 1, last_valid_pos);
    }
    else if(type == 1)
        parseerror(0, cur_linenum - 1, last_valid_pos);
    exit(1);

}

void readToken(string & token, int type) {

    string temp_line;
    if (fin >> token) {

        //if (fin.peek() >= 'a' && fin.peek() <= 'z' || fin.peek() >= 'A' && fin.peek() <= 'Z' || fin.peek() >= '0' && fin.peek() <= '9') {
        if (last_line_offset == -1) {
            cur_offset = fin.tellg() - token.size() + 1;
        }
        else {
            cur_offset = fin.tellg() - last_line_offset - token.size();
        }
        cout << "cur line " << cur_linenum << " cur_offset " << cur_offset << " token " << token << endl;
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

string readSym(int sym_type) {

    string token;
    char c;
    readToken(token, sym_type, c);
    return token;

}

int readInt() {

    string token;
    int type = 1;
    char c;
    readToken(token, type, c);
    int number;
    if (typeCheck(token) == "num") {

        stringstream convert(token);
        convert >> number;
        convert.clear();

    }
    return number;
}
void readDef() {
    int sym_type = 0;
    string sym = readSym(sym_type);
    int val = readInt();
    if (local_module_list.find(sym) == local_module_list.end()) {
        local_module_list[sym] = make_pair(module_num, val + module_addr);
    }
    if (symbol_table.find(sym) == symbol_table.end()) {
        symbol_table[sym] = make_pair(module_num, val + module_addr);
    }
    else {
        symbol_flag[sym] = true; //if sym is met multiple times
    }
}

void readDef2() {
    int sym_type = 0;
    string sym = readSym(sym_type);
    int val = readInt();
}

void readDefList() {
    int numDefs = readInt();
    //cout << "numDefs " << numDefs << endl;
    if (numDefs > 16) {
        ostringstream convert;
        convert << numDefs;
        parseerror(4, cur_linenum, cur_offset - convert.str().length());
        exit(1);
    }
    for (int i = 0; i < numDefs; i++) {
        readDef();
    }
}

void readDefList2() {
    int numDefs = readInt();
    for (int i = 0; i < numDefs; i++) {
        readDef2();
    }
}

void readUse() {
    int sym_type = 0;
    string sym = readSym(sym_type);
}

void readUse2(vector<pair<string, bool>> & uselist) {
    int sym_type = 0;
    string sym = readSym(sym_type);
    uselist.push_back(make_pair(sym, false));
}

void readUseList() {
    int numUse = readInt();
    if (numUse > 16) {
        ostringstream convert;
        convert << numUse;
        parseerror(5, cur_linenum, cur_offset - convert.str().length());
        exit(1);
    }
    for (int i = 0; i < numUse; i++) {
        readUse();
    }
    //cout << "done with use " << endl;
}

void readUseList2(vector<pair<string, bool>> & uselist) {
    int numUse = readInt();
    for (int i = 0; i < numUse; i++) {
        readUse2(uselist);
    }
}

void readInst() {
    int sym_type = 2; // inst
    string sym = readSym(sym_type);
    int val = readInt();
}


void readInstList() {
    int numInst = readInt();
    if (module_addr + numInst > 512) {
        ostringstream convert;
        convert << numInst;
        parseerror(6, cur_linenum, cur_offset - convert.str().length());
        exit(1);
    }
    for (int i = 0; i < numInst; i++) {
        readInst();
    }

    //rule 5
    for (map<string, pair<int, int>>::iterator it = local_module_list.begin(); it != local_module_list.end(); ++it) {
        //cout << "module " << it->first << " addr " << it->second.second << endl;
        if (it->second.second - module_addr > numInst) {
            cout << "Warning: Module " << it->second.first << ": " << it->first << " too big " << it->second.second << " (max=" << numInst-1 << ") assume zero relative" << endl;
            it->second.second = 0 + module_addr;
            symbol_table[it->first].second = 0 + module_addr;
        }
    }
    local_module_list.clear();
    module_addr += numInst;
}

string int2str(int num) {
    ostringstream convert;
    convert << num;
    string temp = convert.str();
    while(temp.length() < 3)
        temp = "0" + temp;
    return temp;
}

string padZero(int num) {
    ostringstream convert;
    convert << num;
    string temp = convert.str();
    while (temp.length() < 4)
        temp = "0" + temp;
    return temp;
}

void readInst2(int cur_num,
               vector<pair<string, bool>> & uselist,
               vector<int> & instlist,
               int numInst) {
    int sym_type = 2;
    string sym = readSym(sym_type);
    int val = readInt();
    //cout << "inst " << sym << " = " << val << endl;
    int result;
    if (sym == "E") {
        int opcode = val / 1000;
        int oprand = val % 1000;
        // rule 11
        if (opcode > 9) {
            cout << int2str(cur_num + module_addr) << ": "  << 9999 << " Error: Illegal opcode; treated as 9999" << endl;
        }
        else {
            // rule 6
            if (oprand >= uselist.size()) {
                cout << int2str(cur_num + module_addr) << ": " << val << " Error: External address exceeds length of uselist; treated as immediate" << endl;
            }
            else {
                string variable = uselist[oprand].first;
                // rule 3
                map<string, pair<int, int>>::iterator it = symbol_table.find(variable);
                if (it == symbol_table.end()) {
                    result = opcode * 1000 + 0;
                    cout << int2str(cur_num + module_addr) << ": " << padZero(result) << " ";
                    cout << "Error: " << variable << " is not defined; zero used" << endl;
                }
                else {
                    result = opcode * 1000 + symbol_table[variable].second;
                    cout << int2str(cur_num + module_addr) << ": " << padZero(result) << endl;
                }
                uselist[oprand].second = true;
            }
        }
    }
    else if (sym == "R") {
        int opcode = val / 1000;
        int oprand = val % 1000;
        // rule 11
        if (opcode > 9) {
            cout << int2str(cur_num + module_addr) << ": "  << 9999 << " Error: Illegal opcode; treated as 9999" << endl;
        }
        else {
            // rule 9
            if (oprand > numInst) {
                result = opcode * 1000 + module_addr;
                cout << int2str(cur_num + module_addr) << ": " << padZero(result) << " Error: Relative address exceeds module size; zero used" << endl;
            }
            else {
                result = opcode * 1000 + oprand + module_addr;
                cout << int2str(cur_num + module_addr) << ": " << padZero(result) << endl;
            }
        }
    }
    else if (sym == "A") {
        int opcode = val / 1000;
        int oprand = val % 1000;
        // rule 8
        if (oprand > 512) {
            oprand = 0;
            result = opcode * 1000 + oprand;
            cout << int2str(cur_num + module_addr) << ": " << padZero(result) << " Error: Absolute address exceeds machine size; zero used" << endl;
        }
        else {
            result = opcode * 1000 + oprand;
            cout << int2str(cur_num + module_addr) << ": " << padZero(result) << endl;
        }

    }
    else {
        result = val;
        if (result > 9999)
            cout << int2str(cur_num + module_addr) << ": " << 9999 << " Error: Illegal immediate value; treated as 9999" << endl;
        else
            cout << int2str(cur_num + module_addr) << ": " << padZero(result) << endl;
    }
    instlist.push_back(result);
}

void readInstList2(vector<pair<string, bool>> & uselist,
                   vector<int> & instlist) {


    int numInst = readInt();
    for (int i = 0; i < numInst; i++) {
        readInst2(i, uselist, instlist, numInst);
    }
    module_addr += numInst;

    // rule 7
    for (int i = 0; i < uselist.size(); i++) {
        if (uselist[i].second == false) {
            cout << "Warning: Module " << module_num << ": " << uselist[i].first << " appeared in the uselist but was not actually used" << endl;
        }
    }


}

void pass1(const string & filename) {

    fin.open(filename.c_str(), ifstream::in);
    cur_linenum = 1;
    cur_offset = 0;
    last_line_offset = -1; //line 1
    module_num = 0;
    //cout << "length of file is " << length << endl;
    module_addr = 0;
    module_global_addr.push_back(module_addr);
    while (fin.peek() != EOF) {
        module_num++;
        //cout << "module " << module_num << endl;
        readDefList();
        readUseList();
        readInstList();
        module_global_addr.push_back(module_addr);
    }
    fin.close();
    cout << "Symbol Table" << endl;
    for (map<string, pair<int, int>>::iterator it = symbol_table.begin(); it != symbol_table.end(); ++it) {

        if (symbol_flag[it->first] == false)
            cout << it->first << "=" << it->second.second << endl;

        if (symbol_flag[it->first] == true) {
            // rule 2
            cout << it->first << "=" << it->second.second;
            cout << " Error: This variable is multiple times defined; first value used" << endl;
        }

    }

}

void pass2(const string & filename) {

    fin.open(filename.c_str(), ifstream::in);
    cur_linenum = 1;
    cur_offset = 0;
    last_line_offset = -1;
    module_num = 0;
    module_addr = 0;

    vector<int> instlist;
    vector<string> all_uselist;
    cout << "Memory Map" << endl;
    while (fin.peek() != EOF) {
        // parse the input if file is open
        module_num++;
        vector<pair<string, bool>> uselist;
        readDefList2();
        readUseList2(uselist);
        readInstList2(uselist, instlist);
        for (auto item : uselist)
            all_uselist.push_back(item.first);
    }
    fin.close();

    // rule 4
    for (auto item : symbol_table) {
        vector<string>::iterator it = find(all_uselist.begin(), all_uselist.end(), item.first);
        if (it == all_uselist.end()) {
            cout << "Warning: Module " << item.second.first  << ": " << item.first << " was defined but never used" << endl;
        }
    }

}

int main(int argc, char *argv[]) {

    string filename = argv[1];
    pass1(filename);
    pass2(filename);
}
