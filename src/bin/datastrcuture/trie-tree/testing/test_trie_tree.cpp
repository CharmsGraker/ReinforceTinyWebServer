#include "../TrieTree.h"
#include <iostream>
using namespace std;
int main() {
    TrieTree<int> trieTree;
    int a = 1;
    int b = 123;
    trieTree.insert("abc",&a);
    trieTree.insert("abcd",&b);

    auto ret =trieTree.contain("abc");
    cout << ret;
    cout << *trieTree["abc"];
    cout << *trieTree["abcd"];
    cout << trieTree[""];

}