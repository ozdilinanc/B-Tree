#pragma once
#include <vector>
#include <string>
#include "BPlusNode.h"

using namespace std;

class BPlusTree
{
private:
    BPlusNode *root; // root node of the B+ tree
    int order;       // maks children per internal node (m) or keys per leaf node (m-1)

    // recursive helper function for insertion (internal worker)
    void insertRecursive(BPlusNode *current, string key, streampos offset, BPlusNode *&newChild, string &promotedKey);

public:
    // Constructor
    BPlusTree(int m);

    // function to insert a key and its file offset into the B+ tree
    void insert(string key, streampos offset);

    // function to search for a key and return the list of file offsets where it appears
    vector<streampos> search(string key);

    bool replaceOffset(string key, streampos oldOffset, streampos newOffset);
    bool removeOffset(string key, streampos oldOffset);
};