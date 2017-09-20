#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>
#include <map>
#include <algorithm>

using namespace std;

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
void readToken(ifstream & fin, int & cur_linenum, int & cur_offset, string & token, string & cur_line) {
    //cout << "I'm currently at " << fin.tellg()<<endl;
    if (cur_offset == -1)
        cur_offset = 0;
    if (cur_offset == 0) {
        getline(fin, cur_line);
        cur_linenum++;
        while (cur_line.empty()){
            getline(fin, cur_line);
            cur_linenum++;
        }
    }
    //cout << cur_offset << endl;
    //cout << cur_line << endl;
    stringstream ss(cur_line);
    ss.seekg(cur_offset, ss.beg);
    ss >> token;
    //cout << "line: " << cur_linenum << " offset: " << ss.tellg() << " token: " << token << endl;
    cur_offset = ss.tellg();
    ss.clear();

}

string readSym(ifstream & fin, int & cur_linenum, int & cur_offset, string & cur_line) {

    string token;
    while (token.empty())
        readToken(fin, cur_linenum, cur_offset, token, cur_line);
//    while (token.empty()) {
//        cout << " ************************************************************************************************" << endl;
//        cout << " *********************sorry, Symbol is empty, you need to read it again**************************" << endl;
//        cout << " ************************************************************************************************" << endl;
//    }

//    if (typeCheck(token) == "sym") {
//        cout << "this is indeed sym" << endl;
//    }
    return token;
}

int readInt(ifstream & fin, int & cur_linenum, int & cur_offset, string & cur_line) {

    string token;
    while (token.empty())
        readToken(fin, cur_linenum, cur_offset, token, cur_line);
//    if (token.empty()) {
//        cout << " ************************************************************************************************" << endl;
//        cout << " *********************sorry, Number is empty, you need to read it again**************************" << endl;
//        cout << " ************************************************************************************************" << endl;
//    }
    int number;
    if (typeCheck(token) == "num") {

        stringstream convert(token);
        convert >> number;
        convert.clear();

    }
    return number;
}
void readDef(ifstream & fin,
             int & cur_linenum,
             int & cur_offset,
             string & cur_line,
             map<string, pair<int,int>> & symbol_table,
             int & module_addr,
             int & module_num) {
    string sym = readSym(fin, cur_linenum, cur_offset, cur_line);
    //cout << "symbol " << sym << endl;
    int val = readInt(fin, cur_linenum, cur_offset, cur_line);

    //rule 5
    if (val > 4) {
        cout << "Warning: Module " << module_num << ": " << sym << " too big " << val << " max=(" << 4 << ") assume zero relative" << endl;
        symbol_table[sym] = make_pair(module_num, 0 + module_addr);
        cout << sym << " = " << 0 + module_addr << endl;
    }
    else {
        //rule 2
        map<string, pair<int,int>>::iterator it = symbol_table.find(sym);
        if (it == symbol_table.end()) {
            symbol_table[sym] = make_pair(module_num, val + module_addr);
            cout << sym << " = " << val + module_addr << " ";
        }
        else {
            cout << "Error: This variable is multiple times defined; first value used" << endl;
        }
        if (it == symbol_table.end())
            cout << endl;
    }

}

void readDef2(ifstream & fin,
             int & cur_linenum,
             int & cur_offset,
             string & cur_line) {
    string sym = readSym(fin, cur_linenum, cur_offset, cur_line);
    int val = readInt(fin, cur_linenum, cur_offset, cur_line);
}


void readDefList(ifstream & fin, int & cur_linenum, int & cur_offset, string & cur_line,
                 map<string, pair<int,int>> & symbol_table,
                 int & module_addr,
                 int & module_num) {
    //cout << "readDefList is called " << endl;
    int numDefs = readInt(fin, cur_linenum, cur_offset, cur_line);
    //cout << "numdef is " << numDefs << endl;
    for (int i = 0; i < numDefs; i++) {
        readDef(fin, cur_linenum, cur_offset, cur_line, symbol_table,module_addr, module_num);
    }
    //cout << "done with def " << endl;
}

void readDefList2(ifstream & fin, int & cur_linenum, int & cur_offset, string & cur_line) {
    //cout << "readDefList is called " << endl;
    int numDefs = readInt(fin, cur_linenum, cur_offset, cur_line);
    //cout << "numdef is " << numDefs << endl;
    for (int i = 0; i < numDefs; i++) {
        readDef2(fin, cur_linenum, cur_offset, cur_line);
    }
    //cout << "done with def " << endl;
}

void readUse(ifstream & fin, int & cur_linenum, int & cur_offset, string & cur_line) {
    string sym = readSym(fin, cur_linenum, cur_offset, cur_line);
    //cout << " used symbol " << sym << endl;
}

void readUse2(ifstream & fin, int & cur_linenum, int & cur_offset, string & cur_line,
              vector<pair<string, bool>> & uselist) {
    string sym = readSym(fin, cur_linenum, cur_offset, cur_line);
    uselist.push_back(make_pair(sym, false));
    //cout << " used symbol " << sym << endl;
}

void readUseList(ifstream & fin, int & cur_linenum, int & cur_offset, string & cur_line) {
    int numUse = readInt(fin, cur_linenum, cur_offset, cur_line);
    //cout << "numUse is " << numUse << endl;
    for (int i = 0; i < numUse; i++) {
        readUse(fin, cur_linenum, cur_offset, cur_line);
    }
    //cout << "done with use " << endl;
}

void readUseList2(ifstream & fin, int & cur_linenum, int & cur_offset, string & cur_line,
                 vector<pair<string, bool>> & uselist) {
    int numUse = readInt(fin, cur_linenum, cur_offset, cur_line);
    //cout << "numUse is " << numUse << endl;
    for (int i = 0; i < numUse; i++) {
        readUse2(fin, cur_linenum, cur_offset, cur_line, uselist);
    }
    //cout << "done with use " << endl;
}

void readInst(ifstream & fin, int & cur_linenum, int & cur_offset, string & cur_line) {
    string sym = readSym(fin, cur_linenum, cur_offset, cur_line);
//    if (sym.empty()) {
//        //cout << "sorry, sym is empty " << endl;
//        return;
//    }
    int val = readInt(fin, cur_linenum, cur_offset, cur_line);
    //cout << "inst " << sym << " = " << val << endl;
}



void readInstList(ifstream & fin, int & cur_linenum, int & cur_offset, string & cur_line, int & module_addr) {
    int numInst = readInt(fin, cur_linenum, cur_offset, cur_line);
    //cout << "numInst is " << numInst << endl;
    for (int i = 0; i < numInst; i++) {
        readInst(fin, cur_linenum, cur_offset, cur_line);
        //cout << " inst " << i << " has finished" << endl;
    }
    //cout << "done with inst " << endl;
    module_addr += numInst;
}

void readInst2(ifstream & fin,
               int & cur_linenum,
               int & cur_offset,
               string & cur_line,
               int cur_num,
               map<string, pair<int,int>> & symbol_table,
               vector<int> & module_global_addr,
               vector<pair<string, bool>> & uselist,
               int & module_addr,
               vector<int> & instruction,
               int numInst) {

    string sym = readSym(fin, cur_linenum, cur_offset, cur_line);
    int val = readInt(fin, cur_linenum, cur_offset, cur_line);
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
                   int & cur_linenum,
                   int & cur_offset,
                   string & cur_line,
                   int & module_addr,
                   map<string, pair<int,int>> & symbol_table,
                   vector<int> & module_global_addr,
                   vector<pair<string, bool>> & uselist,
                   vector<int> & instruction,
                   int module_num) {


    int numInst = readInt(fin, cur_linenum, cur_offset, cur_line);
    //cout << "numInst is " << numInst << endl;
    for (int i = 0; i < numInst; i++) {
        readInst2(fin, cur_linenum, cur_offset, cur_line, i, symbol_table, module_global_addr, uselist, module_addr, instruction,
                  numInst);
        //cout << " inst " << i << " has finished" << endl;
    }
    //cout << "done with inst " << endl;
    module_addr += numInst;

    // rule 7
    for (int i = 0; i < uselist.size(); i++) {
        if (uselist[i].second == false) {
            cout << "Warning: Module " << module_num << " : " << uselist[i].first << " appeared in the uselist but was not actually used" << endl;
        }
    }


}

void pass1(map<string, pair<int,int>> & symbol_table, vector<int> & module_global_addr, string filename) {

    ifstream fin(filename.c_str());
    int cur_linenum = 0;
    int cur_offset = 0;
    string cur_line;
    int module_num = 0;
    fin.seekg(0, fin.end);
    int length = fin.tellg();
    fin.seekg(0, fin.beg);
    //cout << "length of file is " << length << endl;
    int module_addr = 0;
    module_global_addr.push_back(module_addr);
    cout << "Symbol Table" << endl;
    while (!fin.eof()) {
        // parse the input if file is open
        module_num++;
        readDefList(fin, cur_linenum, cur_offset, cur_line, symbol_table, module_addr, module_num);
        readUseList(fin, cur_linenum, cur_offset, cur_line);
        readInstList(fin, cur_linenum, cur_offset, cur_line, module_addr);
        module_global_addr.push_back(module_addr);
        //cout << "module: " << module_num << " done##################################### " << endl;
        if (fin.tellg() == length)
            break;
    }
    //cout << "data has been processed " << endl;

//    for (auto elem : symbol_table)
//        cout << elem.first << " = " << elem.second.second << endl;
    fin.close();

}

void pass2(map<string, pair<int, int>> & symbol_table, vector<int> & module_global_addr, string filename) {

    ifstream fin(filename.c_str());
    int cur_linenum = 0;
    int cur_offset = 0;
    string cur_line;
    int module_num = 0;
    fin.seekg(0, fin.end);
    int length = fin.tellg();
    fin.seekg(0, fin.beg);
    //cout << "length of file is " << length << endl;
    int module_addr = 0;

    vector<int> instruction;
    vector<string> all_uselist;
    cout << "Memory Map" << endl;
    while (!fin.eof()) {
        // parse the input if file is open
        module_num++;
        vector<pair<string, bool>> uselist;
        readDefList2(fin, cur_linenum, cur_offset, cur_line);
        readUseList2(fin, cur_linenum, cur_offset, cur_line, uselist);
        readInstList2(fin, cur_linenum, cur_offset, cur_line, module_addr, symbol_table, module_global_addr, uselist, instruction,
                      module_num);
        //cout << "module: " << module_num << " done##################################### " << endl;
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

    map<string, pair<int, int>> symbol_table;
    vector<int> module_global_addr;

    string filename = "input-11";

    pass1(symbol_table, module_global_addr, filename);


    pass2(symbol_table, module_global_addr, filename);
}
