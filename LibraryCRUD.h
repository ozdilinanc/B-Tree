#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "BPlusTree.h"
#include "StringParser.h"

class LibraryCRUD
{
public:
    BPlusTree idTree;
    BPlusTree authorTree;
    BPlusTree categoryTree;
    BPlusTree booknameTree;
    BPlusTree dateTree;

    LibraryCRUD();

    void insertBook(const Book &book, std::streampos offset);

    std::vector<std::streampos> searchAuthor(const std::string &author);
    std::vector<std::streampos> searchId(const std::string &id);

    bool deleteBook(std::fstream &dbFile, const std::string &id);

    void updateBook(std::fstream &dbFile, const std::string &oldId, const Book &newBook);
};