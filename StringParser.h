#pragma once
#include <string>
#include <vector>

struct Book
{
    std::string id;
    std::string title;
    std::string author;
    std::string category;
    std::string date;
    bool isValid;
};

std::string trim(const std::string &str);
Book parseLine(const std::string &satir);