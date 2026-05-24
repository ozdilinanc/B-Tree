#include "BPlusTree.h"

BPlusTree::BPlusTree(int m)
{
    this->root = new BPlusNode(true);
    this->order = m;
};

void BPlusTree::insert(string key, streampos offset) // insert function over tree
{
}

vector<streampos> BPlusTree::search(string key) // search function over tree
{
    BPlusNode *current = root;

    while (current->isLeafNode == false)
    {
        int i = 0;
        while (i < current->keys.size() && key >= current->keys[i])
        {
            i++;
        }
        current = current->children[i];
    }
}