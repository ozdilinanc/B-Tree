#pragma once
#include <vector>
#include <string>
#include <BPlusNode.h>

using namespace std;

class BPlusTree
{
private:
    BPlusNode *root; // shown the root pointer
    int order;       // maks child number(m)

public:
    BPlusTree(int m); // constructor
    vector<streampos> search(string key);

    void insert(string key, streampos offset);
};