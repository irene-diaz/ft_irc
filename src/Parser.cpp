#include "../include/Parser.hpp"

Parser::Parser()
{
}
Parser::~Parser()
{
}
Parser::Parser(const Parser &other)
{
    (void)other;
}
Parser &Parser::operator=(const Parser &other)
{
    (void)other;
    return *this;
}

std::string Parser::parseCommand(const std::string &line,
                                 std::vector<std::string> &args)
{
    args.clear();

    if (line.empty())
        return "";

    std::stringstream ss(line);

    std::string command;
    if (!(ss >> command))
        return "";

    std::string arg;

    while (ss >> arg)
    {
        if (arg[0] == ':')
        {
            std::string trailing = arg.substr(1);
            std::string rest;

            std::getline(ss, rest);

            if (!rest.empty() && rest[0] == ' ')
                rest.erase(0, 1);

            trailing += rest;
            args.push_back(trailing);
            break;
        }

        args.push_back(arg);
    }

    return command;
}
