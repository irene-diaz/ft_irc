#include "../../include/Server.hpp"

void Server::executeCommand(Client &client, const std::string &command, const std::vector<std::string> &args)
{
    if (command == "PASS")
        handlePass(client, args);
    else if (command == "NICK")
        handleNick(client, args);
    else if (command == "USER")
        handleUser(client, args);
    else if (command == "JOIN")
        handleJoin(client, args);
    else if (command == "PART")
        handlePart(client, args);
    else if (command == "PRIVMSG")
        handlePrivMsg(client, args);
    else if (command == "INVITE")
        handleInvite(client, args);
    else if (command == "TOPIC")
        handleTopic(client, args);
    else if (command == "MODE")
        handleMode(client, args);
    else if (command == "KICK")
        handleKick(client, args);
    else
        std::cerr << "Unknown command: " << command << std::endl;
}



