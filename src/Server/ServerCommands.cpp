#include "../../include/Server.hpp"

void Server::executeCommand(Client &client, const std::string &command, const std::vector<std::string> &args)
{
    if (command == "PASS")
        handlePass(client, args);
    /*else if (command == "NICK")
        handleNick(client, args);
    else if (command == "USER")
        handleUser(client, args);
    else if (command == "JOIN")
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