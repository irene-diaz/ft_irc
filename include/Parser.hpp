#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <sstream>
#include <vector>

class Parser
{
public:
    Parser();
    ~Parser();
    Parser(const Parser &other);
    Parser &operator=(const Parser &other);

    std::string parseCommand(const std::string &line,
                             std::vector<std::string> &args);
};

#endif