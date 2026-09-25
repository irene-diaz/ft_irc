#include "../../include/Server.hpp"
void Server::handleInvite(Client &client,
                          const std::vector<std::string> &args)
{
    if (!checkRegistered(client))
        return;

    if (!checkParams(client, args, 2, "INVITE"))
        return;

    const std::string &nickname = args[0];
    const std::string &channel = args[1];

    std::map<int, Client>::iterator targetIt = _clients.end();
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.getNickname() == nickname)
        {
            targetIt = it;
            break;
        }
    }

    if (targetIt == _clients.end())
    {
        sendNumericReply(client.getFd(), "401", nickname + " :No such nickname");
        return;
    }

    std::map<std::string, Channel>::iterator channelIt = _channels.find(channel);
    if (channelIt == _channels.end())
    {
        sendNumericReply(client.getFd(), "403", channel + " :No such channel");
        return;
    }

    if (!client.isInChannel(channel))
    {
        sendNumericReply(client.getFd(), "442", channel + " :You're not on that channel");
        return;
    }

    if (!channelIt->second.isOperator(client.getFd()))
    {
        sendNumericReply(client.getFd(), "482", channel + " :You're not channel operator");
        return;
    }

    if (targetIt->second.isInChannel(channel))
    {
        sendNumericReply(client.getFd(), "443", nickname + " " + channel + " :is already on channel");
        return;
    }

    channelIt->second.addInvitedClient(targetIt->first);

    std::string inviteMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost INVITE " + nickname + " " + channel + "\r\n";
    sendDataToClient(targetIt->first, inviteMsg);
    sendDataToClient(client.getFd(), inviteMsg);
}