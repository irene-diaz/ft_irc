#include "../../include/Server.hpp"

void Server::executeCommand(Client &client, const std::string &command, const std::vector<std::string> &args)
{
    if (command == "PASS")
        handlePass(client, args);
    else if (command == "NICK")
        handleNick(client, args);
    else if (command == "USER")
        handleUser(client, args);
    /*else if (command == "JOIN")
        handleJoin(client, args);
    else if (command == "PART")
        handlePart(client, args);
    else if (command == "PRIVMSG")
        handlePrivMsg(client, args);*/
    else
        std::cerr << "Unknown command: " << command << std::endl;
}

void Server::handlePass(Client &client, const std::vector<std::string> &args)
{
    if (args.empty())
    {
        sendDataToClient(client.getFd(),
                         "461 PASS :Not enough parameters\r\n");
        return;
    }

    if (args[0] != _password)
    {
        sendDataToClient(client.getFd(),
                         "464 :Password incorrect\r\n");
        return;
    }

    client.setPassAccepted(true);

    std::cout << "Client " << client.getFd()
              << " authenticated successfully." << std::endl;
}

static void Server::tryRegister(Client &client)
{
    if (client.isPassAccepted() && !client.getNickname().empty() && !client.getUsername().empty() && !client.isRegistered())
    {
        client.setRegistered(true);
        std::cout << "Client " << client.getFd()
                  << " registered successfully." << std::endl;
    }
}

void Server::handleNick(Client &client, const std::vector<std::string> &args)
{
    if (!client.isPassAccepted())
    {
        sendDataToClient(client.getFd(),
                         "464 :You must authenticate first\r\n");
        return;
    }

    if (args.empty())
    {
        sendDataToClient(client.getFd(),
                         "431 :No nickname given\r\n");
        return;
    }

    for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.getNickname() == args[0])
        {
            sendDataToClient(client.getFd(),
                             "433 :Nickname is already in use\r\n");
            return;
        }
    }

    client.setNickname(args[0]);
    std::cout << "Client " << client.getFd()
              << " set nickname to: " << args[0] << std::endl;

    tryRegister(client);
}

void Server::handleUser(Client &client, const std::vector<std::string> &args)
{
    if (!client.isPassAccepted())
    {
        sendDataToClient(client.getFd(),
                         "464 :You must authenticate first\r\n");
        return;
    }

    if (args.size() < 4)
    {
        sendDataToClient(client.getFd(),
                         "461 USER :Not enough parameters\r\n");
        return;
    }

    client.setUsername(args[0]);
    client.setRealname(args[3]);

    std::cout << "Client " << client.getFd()
              << " registered with username: " << args[0]
              << " and realname: " << args[3] << std::endl;

    tryRegister(client);
}