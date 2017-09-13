#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <regex>
using namespace std;

vector<string> split(string &s){
    stringstream ss(s);
    string item;
    vector<string> tokens;

    while (ss >> item)
        tokens.push_back(item);
    return tokens;
}

bool existsInDef(vector<vector<pair<string, int>>>& defcountlist, string symbol, int& modnum, int& symbnum) {
    for (int i = 0; i < defcountlist.size(); i++) {
        for (int j = 0; j < defcountlist[i].size(); j++) {
            if (symbol == defcountlist[i][j].first) {
                modnum = i;
                symbnum = j;
                return true;
            }
        }
    }
    return false;
}

void parser_1(vector<vector<pair<string, int>>>& defcountlist, vector<vector<string>>& usecountlist,vector<vector<pair<string, string>>>& codecountlist,vector<int>& globaddr) {

    fstream fin("input-11", fstream::in);
    //fstream fout("output-2", fstream::out);
    string line;
    string everything;
    int linenum = 0;
    int lineoffset = 0;
    if (fin.is_open()) {
        while (getline(fin, line)) {
            linenum += 1;
            vector<string> space_split = split(line);
            for (int i = 0; i < space_split.size(); i++){
                string token = space_split[i];
                if (regex_match(token, regex("[[:alpha:]][[:alnum:]]{0,15}"))) {
                    if (token != "I" && token != "A" && token != "R" && token != "E") {
                        //cout << "symbol " << space_split[i] << " ";
                        //everything += "symbol ";
                        everything += space_split[i];
                        everything += " ";
                    }
                    else {
                        //cout << "instr " << space_split[i] << " ";
                        //everything += "instr ";
                        everything += space_split[i];
                        everything += " ";
                    }
                }
                else if (regex_match(token, regex("[[:digit:]][[:digit:]]*"))) {
                    //cout << "number " << space_split[i] << " ";
                    //everything += "number ";
                    everything += space_split[i];
                    everything += " ";
                }

            }

        }
        cout << everything << endl;
        vector<string> sep = split(everything);


        int i = 0;
        //cout << "size of tokens " << sep.size() << endl;
        int glob = 0;

        while (i < sep.size()) {
            //start with deflist, then uselist, then program text
            //cout << "+" << glob << endl;
            globaddr.push_back(glob);

            string token = sep[i];
            int defcount = 0;
            stringstream convert;
            convert.str(token);
            convert >> defcount;
            //cout << "defcount is " << defcount << endl;
            int j = i+1;
            vector<pair<string,int>> deftemp;
            while (j < i+1+defcount*2) {
                int addr = 0;
                convert.clear();
                convert.str(sep[j+1]);
                convert >> addr;
                int modnum = 0;
                int symbnum = 0;
                if (existsInDef(defcountlist, sep[j], modnum, symbnum)) {
                    //cout << modnum << " " << symbnum << endl;
                    cout << sep[j] << "=" << defcountlist[modnum][symbnum].second << " Error: This variable is multiple times defined; first value used" << endl;
                    break;
                }
                deftemp.push_back(make_pair(sep[j], addr + glob));
                //cout << sep[j] << " = " << addr + glob << endl;
                j += 2;
            }
            defcountlist.push_back(deftemp);
            //cout << "defcount printed" << endl;
            i += (1 + defcount * 2);
            token = sep[i];
            int usecount = 0;
            convert.clear();
            convert.str(token);
            convert >> usecount;
            //cout << "usecount is " << usecount << endl;
            j = i+1;
            vector<string> usetemp;
            while (j < i+1+usecount) {
                usetemp.push_back(sep[j]);
                //cout << sep[j] << " ";
                j += 1;
            }
            //cout << endl;
            usecountlist.push_back(usetemp);
            //cout << "usecound printed" << endl;
            i += (1 + usecount);
            token = sep[i];
            int codecount = 0;
            convert.clear();
            convert.str(token);
            convert >> codecount;
            j = i+1;
            vector<pair<string, string>> codetemp;
            while (j < i+1+codecount*2) {
                codetemp.push_back(make_pair(sep[j], sep[j+1]));
                //cout << sep[j] << " " << sep[j+1] << endl;
                j += 2;
            }
            codecountlist.push_back(codetemp);
            //cout << "defcount printed" << endl;
            i += (1 + codecount * 2);
            glob += codecount;
        }


        fin.close();
        //fout.close();
    }
}

void parser_2(vector<vector<pair<string, int>>> defcountlist, vector<vector<string>> usecountlist, vector<vector<pair<string, string>>> codecountlist,vector<int> globaddr) {

    vector<vector<bool>> def_flag(defcountlist.size());
    for (int i = 0; i < defcountlist.size(); i++) {
        vector<bool> temp_flag(defcountlist[i].size(), false);
        def_flag[i] = temp_flag;
    }

    vector<vector<bool>> use_flag(usecountlist.size());
    for (int i = 0; i < usecountlist.size(); i++) {
        vector<bool> temp_flag(usecountlist[i].size(), false);
        use_flag[i] = temp_flag;
    }

    cout << "Symbol Table" << endl;
    for (int i = 0; i < defcountlist.size(); i++) {
        for (int j = 0; j < defcountlist[i].size(); j++) {
            if (defcountlist[i][j].second > codecountlist[i].size()) {
                cout << "Warning: Module " << i+1 << ": " << defcountlist[i][j].first << " too big " << defcountlist[i][j].second << " (max=" << codecountlist[i].size() - 1 << ") assume zero relative" << endl;
                defcountlist[i][j].second = 0;
            }
            cout << defcountlist[i][j].first << " = " << defcountlist[i][j].second << endl;
        }
    }
    cout << "Memory Map" << endl;

    int c = 0;
    for (int modcount = 0; modcount < globaddr.size(); modcount++) {
        vector<pair<string,string>> temp;
        temp = codecountlist[modcount];
        int X = globaddr[modcount];
        vector<string> usecount = usecountlist[modcount];
        string symbol("none");
        for (int i = 0; i < temp.size(); i++) {
            string opcode = temp[i].first;
            string oprand = temp[i].second;
            int addr = 0;
            stringstream convert;
            convert.clear();
            convert.str(oprand);
            convert >> addr;
            if (opcode == "R") {
                addr += X;
                cout << c << " " << addr << endl;
            }
            else if (opcode == "I") {
                addr = addr;
                cout << c << " " << addr << endl;
            }
            else if (opcode == "A") {
                addr = addr;
                cout << c << " " << addr << endl;
            }
            else {
                int mod = addr % 1000;
                if (mod >= usecount.size()) {
                    cout << c << " " << addr << " Error: External address exceeds length of uselist; treated as immediate" << endl;
                    c += 1;
                    continue;
                }
                symbol = usecount[mod];
                use_flag[modcount][mod] = true;
                for (int k = 0; k < defcountlist.size(); k++) {
                    for (int l = 0; l < defcountlist[k].size(); l++) {
                        if (symbol == defcountlist[k][l].first) {
                            int disp = defcountlist[k][l].second;
                            addr -= mod;
                            addr += disp;
                            def_flag[k][l] = true;
                        }
                    }

                }
                int modnum = 0;
                int symbnum = 0;
                if (existsInDef(defcountlist, symbol, modnum, symbnum))
                    cout << c << " " << addr << endl;
                else
                    cout << c << " " << addr << " Error: " << symbol << " is not defined; zero used" << endl;

            }
            c += 1;

        }
        //rule 7
        for (int l = 0; l < use_flag[modcount].size(); l++) {
            if (use_flag[modcount][l] == false) {
                cout << "Warning: Module " << modcount+1 << ": " << usecountlist[modcount][l] << " appeared in the uselist but was not actually used" << endl;
            }
        }
    }
    // rule 4
    for (int k = 0; k < def_flag.size(); k++) {
        for (int l = 0; l < def_flag[k].size(); l++) {
            if (def_flag[k][l] == false) {
                cout << "Warning: Module " << k+1 << ": " << defcountlist[k][l].first << " was defined but never used" << endl;
            }
        }
    }
}

int main() {

    cout << "This is a good start" << endl;
    vector<vector<pair<string, int>>> defcountlist;
    vector<vector<string>> usecountlist;
    vector<vector<pair<string, string>>> codecountlist;
    vector<int> globaddr;
    cout << "Parser_1 is called" << endl;
    parser_1(defcountlist, usecountlist,codecountlist,globaddr);
    cout << "Parser_2 is called" << endl;
    parser_2(defcountlist, usecountlist,codecountlist,globaddr);

    return 1;
}


