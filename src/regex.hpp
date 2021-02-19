#ifndef REGEX_HPP
#define REGEX_HPP

#include <regex>
#include <string>

class Regex
{
public:
    std::regex rgx;
    std::regex_iterator<std::string::iterator> it;

    short color;

    Regex(std::string rule, std::string* line, short color);
};

#endif
