#include "LibraryCRUD.h"
#include <iostream>

using namespace std;

LibraryCRUD::LibraryCRUD()
    : idTree(50), authorTree(50), categoryTree(50), booknameTree(50), dateTree(50) {}

void LibraryCRUD::insertBook(const Book &book, streampos offset)
{
    idTree.insert(book.id, offset);
    authorTree.insert(book.author, offset);
    categoryTree.insert(book.category, offset);
    booknameTree.insert(book.title, offset);
    dateTree.insert(book.date, offset);
}

vector<streampos> LibraryCRUD::searchAuthor(const string &author)
{
    return authorTree.search(author);
}

vector<streampos> LibraryCRUD::searchId(const string &id)
{
    return idTree.search(id);
}

bool LibraryCRUD::deleteBook(fstream &dbFile, const string &id)
{
    vector<streampos> offsets = idTree.search(id);
    if (offsets.empty())
        return false;

    streampos offset = offsets[0];
    dbFile.clear();
    dbFile.seekp(offset);
    dbFile.put('*'); // Mantıksal (Soft) Silme işareti koy

    return true;
}

void LibraryCRUD::updateBook(fstream &dbFile, const string &oldId, const Book &newBook)
{
    deleteBook(dbFile, oldId);

    dbFile.clear();
    dbFile.seekp(0, ios::end);
    streampos newOffset = dbFile.tellp();

    dbFile << newBook.id << ", " << newBook.title << ", "
           << newBook.author << ", " << newBook.category << ", " << newBook.date << "\r\n";

    insertBook(newBook, newOffset);
}