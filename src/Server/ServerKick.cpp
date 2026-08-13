#include "../../include/Server.hpp"

void Server::handleKick(Client &client,
                        const std::vector<std::string> &args)
{
    if (!checkRegistered(client))
        return;

    if (!checkParams(client, args, 2, "KICK"))
        return;

    const std::string &channel = args[0];
    const std::string &nickname = args[1];

    // Check if the channel exists
    std::map<std::string, Channel>::iterator channelIt = _channels.find(channel);
    if (channelIt == _channels.end())
    {
        sendNumericReply(client.getFd(), "403", channel + " :No such channel");
        return;
    }

    // The client performing KICK must be in the channel
    if (!client.isInChannel(channel))
    {
        sendNumericReply(client.getFd(), "442", channel + " :You're not on that channel");
        return;
    }

    // The client performing KICK must be an operator
    if (!channelIt->second.isOperator(client.getFd()))
    {
        sendNumericReply(client.getFd(), "482", channel + " :You're not channel operator");
        return;
    }

    // Find the target client
    Client *targetClient = NULL;

    for (std::map<int, Client>::iterator it = _clients.begin();
         it != _clients.end();
         ++it)
    {
        if (it->second.getNickname() == nickname)
        {
            targetClient = &it->second;
            break;
        }
    }

    // Target nickname doesn't exist
    if (targetClient == NULL)
    {
        sendNumericReply(client.getFd(),
                         "401",
                         nickname + " :No such nickname");
        return;
    }

    // Target is not in the channel
    if (!targetClient->isInChannel(channel))
    {
        sendNumericReply(client.getFd(),
                         "441",
                         nickname + " " + channel + " :They aren't on that channel");
        return;
    }

    std::string reply = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost KICK " + channel + " " + nickname + "\r\n";

    // Notify everyone in the channel, including the target
    for (std::map<int, Client>::iterator it = _clients.begin();
         it != _clients.end();
         ++it)
    {
        if (it->second.isInChannel(channel))
            sendDataToClient(it->first, reply);
    }

    // Remove the target from both sides
    targetClient->partChannel(channel);                    // modify the client
    channelIt->second.removeClient(targetClient->getFd()); // modify the channel

    std::cout << "Client " << client.getFd()
              << " kicked "
              << nickname
              << " from "
              << channel << std::endl;
}