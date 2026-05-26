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

    // variables to hold the new child and promoted key if the root splits
    BPlusNode *newChild = nullptr; // if the root splits, this will point to the right half of the split
    string promotedKey = "";       // if the root splits, this will hold the key which is promoted to the upper level (new root)

    // start recursive insertion from the root to find the correct leaf node and insert the key there
    insertRecursive(root, key, offset, newChild, promotedKey);

    // if after recursive insertion, we have a new child (meaning the root was split), we need to create a new root
    if (newChild != nullptr)
    {
        BPlusNode *newRoot = new BPlusNode(false); // create new root node (internal node)
        newRoot->keys.push_back(promotedKey);      // promote the key that was pushed up from the old root
        newRoot->children.push_back(root);         // left child of the new root is the old root
        newRoot->children.push_back(newChild);     // right child of the new root is the new child
        root = newRoot;                            // update the root pointer to point to the new root
    }
}

// RECURSIVE INSERTION HELPER FUNCTION IMPLEMENTATION
void BPlusTree::insertRecursive(BPlusNode *current, string key, streampos offset, BPlusNode *&newChild, string &promotedKey)
{
    newChild = nullptr; // default no new child, no promoted key

    // base case: if it's a leaf node, insert the key and offset here
    if (current->isLeafNode)
    {
        // 1.if the key already exists in this leaf, just add the offset to the existing list
        for (int i = 0; i < current->keys.size(); i++)
        {
            if (current->keys[i] == key)
            {
                current->dataOffsets[i].push_back(offset); // add the new offset to the existing list for this key
                return;
            }
        }

        // 2. if the key does not exist, insert it in sorted order along with its offset
        int pos = 0; // creaate a position variable to keep track of where to insert the new key in sorted order

        while (pos < current->keys.size() && key > current->keys[pos]) // loop to find the correct position to insert the new key in sorted order
            pos++;

        current->keys.insert(current->keys.begin() + pos, key);                         // begin(address of the first element) + pos to insert the new key in sorted order
        vector<streampos> newOffsetList;                                                // create a new vector to hold the offset for this new key
        newOffsetList.push_back(offset);                                                // add the offset to the new vector
        current->dataOffsets.insert(current->dataOffsets.begin() + pos, newOffsetList); // insert the new offset vector at the same position as the new key

        // 3. after while loop, we have inserted the new key and its offset. Now we need to check size of the keys in this leaf node.
        if (current->keys.size() == order) // if the keys size is equal to the order, so we need to split this leaf node. else we are done and can return.
        {
            BPlusNode *newNode = new BPlusNode(true); // right half of the split will be a new leaf node
            int mid = order / 2;                      // find the middle index to split the keys and offsets

            for (int i = mid; i < order; i++)
            {
                newNode->keys.push_back(current->keys[i]);               // move the keys from the old leaf to the new right leaf
                newNode->dataOffsets.push_back(current->dataOffsets[i]); // move the corresponding offsets to the new right leaf
            }

            current->keys.erase(current->keys.begin() + mid, current->keys.end());                      // remove the keys that were moved to the new node from the old leaf
            current->dataOffsets.erase(current->dataOffsets.begin() + mid, current->dataOffsets.end()); // remove the offsets that were moved to the new node from the old leaf

            // connect the new leaf node to the existing leaf nodes
            newNode->next = current->next; // connect the new node's next pointer to the current node's next (the old leaf's next)
            current->next = newNode;       // connect the current node's next pointer to the new node (the old leaf's next now points to the new leaf)

            promotedKey = newNode->keys[0]; // the first key of the new right leaf will be promoted to the parent (internal node) to act as a separator between the two leaf nodes
            newChild = newNode;             // set the new child pointer to point to the new right leaf node,
        }
        return;
    }

    // if it's not a leaf node, we need to find the correct child to go down to insert the key
    int i = 0;
    while (i < current->keys.size() && key >= current->keys[i]) // loop to find the correct child pointer to follow based on the key values in the internal node
        i++;

    BPlusNode *childToInsert = current->children[i]; // we choose to go down to this child node to continue the insertion process.

    // call the recursive insertion on the child node we chose to go down to. This will continue until we reach a leaf node and insert the key there.
    insertRecursive(childToInsert, key, offset, newChild, promotedKey);

    // we return back up the recursive calls. if newChild is still nullptr return
    if (newChild == nullptr)
        return;

    // if newChild is not nullptr, it means the child node we went down to was split and we have a new child node and a promoted key
    int pos = 0;
    while (pos < current->keys.size() && promotedKey > current->keys[pos]) // loop to find the correct position to insert the promoted key
        pos++;

    current->keys.insert(current->keys.begin() + pos, promotedKey);          // insert the promoted key into the current internal node
    current->children.insert(current->children.begin() + pos + 1, newChild); // insert the new child pointer into the current internal node

    // 4. after inserting the promoted key and new child pointer, we need to check if the current internal node has too many keys and needs to be split
    if (current->keys.size() == order)
    {
        BPlusNode *newNode = new BPlusNode(false); // right half of the split will be a new internal node
        int mid = order / 2;                       // find the middle index to split the keys and child pointers

        string newPromotedKey = current->keys[mid]; // the middle key will be promoted to the parent of the current internal node

        for (int j = mid + 1; j < order; j++) // move the keys from the old internal node to the new right internal node (starting from mid + 1 because mid is promoted)
        {
            newNode->keys.push_back(current->keys[j]);         // move the keys from the old internal node to the new right internal node
            newNode->children.push_back(current->children[j]); // move the corresponding child pointers to the new right internal node
        }
        newNode->children.push_back(current->children[order]); // we also need to move the last child pointer (the one after the last key) to the new right internal node

        current->keys.erase(current->keys.begin() + mid, current->keys.end());                 // remove the keys that were moved to the new node from the old internal node
        current->children.erase(current->children.begin() + mid + 1, current->children.end()); // remove the child pointers that were moved to the new node from the old internal node

        promotedKey = newPromotedKey; // set the promoted key to be the middle key that we found earlier
        newChild = newNode;           // set the new child pointer to point to the new right internal node that we created from the split
    }
    else
    {
        newChild = nullptr; // if we did not split the current internal node, we do not have a new child to return up the recursive calls, so we set newChild to nullptr
    }
}

// SEARCH FUNCTION IMPLEMENTATION
vector<streampos> BPlusTree::search(string key)
{
    BPlusNode *current = root;

    // traverse down the tree to find the correct leaf node where the key should be located
    while (current->isLeafNode == false)
    {
        int i = 0;
        while (i < current->keys.size() && key >= current->keys[i])
        {
            i++;
        }
        current = current->children[i]; // follow the child pointer to go down to the next level of the tree based on the key values in the internal node
    }

    // we are now at the leaf node where the key should be located if it exists. we need to search through the keys in this leaf node to find the key and return its associated offsets
    for (int i = 0; i < current->keys.size(); i++)
    {
        if (current->keys[i] == key)
        {
            return current->dataOffsets[i];
        }
    }

    // if we did not find the key in the leaf node, it means the key does not exist in the tree, so we return an empty vector to indicate that the key was not found
    return {};
}

// Doğrudan Adres Güncelleme (In-Place Pointer Update)
bool BPlusTree::replaceOffset(string key, streampos oldOffset, streampos newOffset)
{
    BPlusNode *current = root;
    while (!current->isLeafNode)
    {
        int i = 0;
        while (i < current->keys.size() && key >= current->keys[i])
            i++;
        current = current->children[i];
    }
    for (int i = 0; i < current->keys.size(); i++)
    {
        if (current->keys[i] == key)
        {
            for (int j = 0; j < current->dataOffsets[i].size(); j++)
            {
                if (current->dataOffsets[i][j] == oldOffset)
                {
                    current->dataOffsets[i][j] = newOffset; // Adresi direkt değiştir!
                    return true;
                }
            }
        }
    }
    return false;
}

// Eski Adresi Ağaçtan Temizleme
bool BPlusTree::removeOffset(string key, streampos oldOffset)
{
    BPlusNode *current = root;
    while (!current->isLeafNode)
    {
        int i = 0;
        while (i < current->keys.size() && key >= current->keys[i])
            i++;
        current = current->children[i];
    }
    for (int i = 0; i < current->keys.size(); i++)
    {
        if (current->keys[i] == key)
        {
            for (auto it = current->dataOffsets[i].begin(); it != current->dataOffsets[i].end(); ++it)
            {
                if (*it == oldOffset)
                {
                    current->dataOffsets[i].erase(it); // Çöp adresi sil!
                    return true;
                }
            }
        }
    }
    return false;
}