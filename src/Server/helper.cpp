#include "../../include/Server.hpp"

bool Server::isValidNickname(const std::string &nickname) const
{
    if (nickname.empty())
        return false;

    // First character
    char c = nickname[0];

    if (!std::isalpha(c) &&
        c != '[' && c != ']' &&
        c != '\\' && c != '`' &&
        c != '^' && c != '_' &&
        c != '{' && c != '|' &&
        c != '}')
    {
        return false;
    }

    // Remaining characters
    for (size_t i = 1; i < nickname.size(); ++i)
    {
        c = nickname[i];

        if (!std::isalnum(c) &&
            c != '-' &&
            c != '[' && c != ']' &&
            c != '\\' && c != '`' &&
            c != '^' && c != '_' &&
            c != '{' && c != '|' &&
            c != '}')
        {
            return false;
        }
    }

    return true;
}