#include "BPlusTree.h"

BPlusTree::BPlusTree(int m)
{
    root = new BPlusNode(true);
    order = m;
};

void BPlusTree::insert(string key, streampos offset)
{
    // insert function over tree
}

vector<streampos> BPlusTree::search(string key)
{
    // search function over tree
    return {};
}