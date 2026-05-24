#include "BPlusTree.h"

// CONSTRUCTOR IMPLEMENTATION
BPlusTree::BPlusTree(int m)
{
    this->root = new BPlusNode(true);
    this->order = m;
}

// INSERT FUNCTION IMPLEMENTATION
void BPlusTree::insert(string key, streampos offset)
{
    // if the tree is empty, insert the first key directly into the root
    if (root->keys.empty())
    {
        root->keys.push_back(key);                    // add the key to the root
        vector<streampos> firstOffsetList;            // create a new vector for this key to hold offsets
        firstOffsetList.push_back(offset);            // add the offset to the vector
        root->dataOffsets.push_back(firstOffsetList); // associate the offset vector with the key in the root
        return;
    }

    // Aşağıdan (yapraktan) yukarıya dönecek olan "Bölünme Mesajları"
    BPlusNode *newChild = nullptr;
    string promotedKey = "";

    // Recursive işlemi başlat (Kökten aşağı iniyoruz)
    insertRecursive(root, key, offset, newChild, promotedKey);

    // EĞER KÖK (ROOT) BÖLÜNDÜYSE: Yeni bir Root yaratmalıyız
    if (newChild != nullptr)
    {
        BPlusNode *newRoot = new BPlusNode(false); // Yeni root iç düğümdür
        newRoot->keys.push_back(promotedKey);
        newRoot->children.push_back(root);     // Eski root sola geçti
        newRoot->children.push_back(newChild); // Yeni yarılan parça sağa geçti
        root = newRoot;
    }
}

// RECURSIVE INSERTION HELPER FUNCTION IMPLEMENTATION
void BPlusTree::insertRecursive(BPlusNode *current, string key, streampos offset, BPlusNode *&newChild, string &promotedKey)
{
    newChild = nullptr; // default no new child, no promoted key

    // base case: if it's a leaf node, insert the key and offset here
    if (current->isLeafNode)
    {
        // 1. if the key already exists in this leaf, just add the offset to the existing list
        for (int i = 0; i < current->keys.size(); i++)
        {
            if (current->keys[i] == key)
            {
                current->dataOffsets[i].push_back(offset);
                return;
            }
        }

        int pos = 0;
        while (pos < current->keys.size() && key > current->keys[pos])
            pos++;

        current->keys.insert(current->keys.begin() + pos, key);
        vector<streampos> newOffsetList;
        newOffsetList.push_back(offset);
        current->dataOffsets.insert(current->dataOffsets.begin() + pos, newOffsetList);

        if (current->keys.size() == order)
        {
            BPlusNode *newNode = new BPlusNode(true);
            int mid = order / 2;

            for (int i = mid; i < order; i++)
            {
                newNode->keys.push_back(current->keys[i]);
                newNode->dataOffsets.push_back(current->dataOffsets[i]);
            }

            current->keys.erase(current->keys.begin() + mid, current->keys.end());
            current->dataOffsets.erase(current->dataOffsets.begin() + mid, current->dataOffsets.end());

            newNode->next = current->next;
            current->next = newNode;

            promotedKey = newNode->keys[0];
            newChild = newNode;
        }
        return;
    }

    int i = 0;
    while (i < current->keys.size() && key >= current->keys[i])
        i++;

    BPlusNode *childToInsert = current->children[i];

    insertRecursive(childToInsert, key, offset, newChild, promotedKey);

    if (newChild == nullptr)
        return;

    int pos = 0;
    while (pos < current->keys.size() && promotedKey > current->keys[pos])
        pos++;

    current->keys.insert(current->keys.begin() + pos, promotedKey);
    current->children.insert(current->children.begin() + pos + 1, newChild);

    if (current->keys.size() == order)
    {
        BPlusNode *newNode = new BPlusNode(false);
        int mid = order / 2;

        string newPromotedKey = current->keys[mid];

        for (int j = mid + 1; j < order; j++)
        {
            newNode->keys.push_back(current->keys[j]);
            newNode->children.push_back(current->children[j]);
        }
        newNode->children.push_back(current->children[order]);

        current->keys.erase(current->keys.begin() + mid, current->keys.end());
        current->children.erase(current->children.begin() + mid + 1, current->children.end());

        promotedKey = newPromotedKey;
        newChild = newNode;
    }
    else
    {
        newChild = nullptr;
    }
}

// SEARCH FUNCTION IMPLEMENTATION
vector<streampos> BPlusTree::search(string key)
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

    for (int i = 0; i < current->keys.size(); i++)
    {
        if (current->keys[i] == key)
        {
            return current->dataOffsets[i];
        }
    }

    return {};
}