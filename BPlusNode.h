#pragma once
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class BPlusNode
{
public:
    bool isLeafNode;                       // check if it's a leaf node or not
    vector<string> keys;                   // holds the keys (for both internal and leaf nodes)
    vector<BPlusNode *> children;          // only for internal nodes: pointers to child nodes
    vector<vector<streampos>> dataOffsets; // only for leaf nodes: holds the list of file offsets for each key (multiple entries for duplicate keys)
    BPlusNode *next;                       // only for leaf nodes: point to the next leaf node

    // Constructor
    BPlusNode(bool isleaf)
    {
        isLeafNode = isleaf;
        next = nullptr;
    }
};