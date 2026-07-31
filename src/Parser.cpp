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

std::string Parser::parseCommand(const std::string &line, std::vector<std::string> &args)
{
    args.clear(); // Clear the args vector before parsing

    if (line.empty())
        return "";

    std::stringstream ss(line); // Create a stringstream to parse the line

    std::string command; // Variable to hold the command extracted from the line
    if (!(ss >> command))
        return "";

    std::string arg; // Variable to hold each argument extracted from the line

    while (ss >> arg)
        args.push_back(arg); // Store each argument in the args vector

    // For debugging: print the command and its arguments
    std::cout << "Command: " << command << std::endl;
    for (size_t i = 0; i < args.size(); ++i)
        std::cout << "Arg " << i << ": " << args[i] << std::endl;
    return command; // Return the parsed command
}
