#include "../../include/Server.hpp"

void Server::tryRegister(Client &client)
{
    if (client.isPassAccepted() && !client.getNickname().empty() && !client.getUsername().empty() && !client.isRegistered())
    {
        client.setRegistered(true);

        std::cout << "Client " << client.getFd()
                  << " registered successfully." << std::endl;

        sendNumericReply(client.getFd(),
                         "001",
                         client.getNickname() + " :Welcome to the IRC server");
    }
}

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

// Check if the client is registered before executing certain commands
bool Server::checkRegistered(Client &client)
{
    if (!client.isRegistered())
    {
        sendNumericReply(client.getFd(),
                         "451",
                         ":You have not registered");
        return false;
    }

    return true;
}

bool Server::checkParams(Client &client,
                         const std::vector<std::string> &args,
                         size_t minimum,
                         const std::string &command)
{
    if (args.size() < minimum)
    {
        sendNumericReply(client.getFd(),
                         "461",
                         command + " :Not enough parameters");
        return false;
    }

    return true;
}