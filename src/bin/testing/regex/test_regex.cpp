#include <regex>
#include <iostream>
using namespace std;
int main() {
    std::string str ="1usr/local";
    std::regex txt_regex("(.*)local$");
    smatch results;
    cout << std::regex_match(str,results,txt_regex) << endl;
    // smatch[0] store the whole raw matched str
    // add '()' will store in each ele in smatch
    cout << results[0]<< endl;
    cout << results[1]<< endl;
}