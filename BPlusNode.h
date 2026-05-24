#pragma once
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class BPlusNode
{

public:
    bool isLeafNode;              // controt the is it a leaf node or inner node
    vector<string> keys;          // hold the key values
    vector<BPlusNode *> children; // if its inner node then point the children nodes

    vector<vector<streampos>> dataOffsets; // holds the keys value which contain different streamposs values
    BPlusNode *next;

    BPlusNode(bool leaf)
    {
        isLeafNode = leaf;
        next = nullptr; // make next pointer empty because new node on the far right
    }
};