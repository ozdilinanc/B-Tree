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

vector<streampos> LibraryCRUD::searchCategory(const string &category)
{
    return categoryTree.search(category);
}
vector<streampos> LibraryCRUD::searchBookName(const string &bookName)
{
    return booknameTree.search(bookName);
}

vector<streampos> LibraryCRUD::searchDate(const string &date)
{
    return dateTree.search(date);
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
    vector<streampos> offsets = idTree.search(oldId);
    if (offsets.empty())
    {
        cout << "No record found.\n";
        return;
    }

    streampos oldOffset = -1;
    string oldLine;

    // Adım 1: Aktif (silinmemiş) kaydın adresini bul
    for (streampos off : offsets)
    {
        dbFile.clear();
        dbFile.seekg(off);
        getline(dbFile, oldLine);
        if (!oldLine.empty() && oldLine[0] != '*')
        {
            oldOffset = off;
            break;
        }
    }
    if (oldOffset == -1)
        return;

    // Eski veriyi parçala (Yazar veya Kategori isminin değişip değişmediğini anlamak için)
    Book oldBook = parseLine(oldLine);

    // Adım 2: Diskteki eski kaydı sil (*)
    dbFile.clear();
    dbFile.seekp(oldOffset);
    dbFile.put('*');

    // Adım 3: Diskin sonuna yeni kaydı yaz ve yeni adresi (newOffset) al
    dbFile.clear();
    dbFile.seekp(0, ios::end);
    streampos newOffset = dbFile.tellp();
    dbFile << newBook.id << ", " << newBook.title << ", "
           << newBook.author << ", " << newBook.category << ", " << newBook.date << "\r\n";

    idTree.replaceOffset(oldBook.id, oldOffset, newOffset);

    if (oldBook.author == newBook.author)
        authorTree.replaceOffset(oldBook.author, oldOffset, newOffset);
    else
    {
        authorTree.removeOffset(oldBook.author, oldOffset);
        authorTree.insert(newBook.author, newOffset);
    }

    if (oldBook.category == newBook.category)
        categoryTree.replaceOffset(oldBook.category, oldOffset, newOffset);
    else
    {
        categoryTree.removeOffset(oldBook.category, oldOffset);
        categoryTree.insert(newBook.category, newOffset);
    }

    if (oldBook.title == newBook.title)
        booknameTree.replaceOffset(oldBook.title, oldOffset, newOffset);
    else
    {
        booknameTree.removeOffset(oldBook.title, oldOffset);
        booknameTree.insert(newBook.title, newOffset);
    }

    if (oldBook.date == newBook.date)
        dateTree.replaceOffset(oldBook.date, oldOffset, newOffset);
    else
    {
        dateTree.removeOffset(oldBook.date, oldOffset);
        dateTree.insert(newBook.date, newOffset);
    }
}