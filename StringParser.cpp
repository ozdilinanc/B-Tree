#include "StringParser.h"
#include <sstream>

using namespace std;

string trim(const string &str)
{
    size_t ilk = str.find_first_not_of(" \t\r\n");
    if (ilk == string::npos)
        return "";
    size_t son = str.find_last_not_of(" \t\r\n");
    return str.substr(ilk, (son - ilk + 1));
}

Book parseLine(const string &satir)
{
    Book book;
    book.isValid = false;
    vector<string> parcalar;
    stringstream ss(satir);
    string parca;

    while (getline(ss, parca, ','))
    {
        parcalar.push_back(trim(parca));
    }

    if (parcalar.size() >= 5)
    {
        int n = parcalar.size();
        book.id = parcalar[0];
        book.date = parcalar[n - 1];
        book.category = parcalar[n - 2];
        book.author = parcalar[n - 3];

        book.title = parcalar[1];
        for (int i = 2; i <= n - 4; ++i)
        {
            book.title += "," + parcalar[i];
        }
        if (book.id[0] == '*')
            book.isValid = false;
        else
            book.isValid = true;
    }
    return book;
}