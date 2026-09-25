#include "../../include/Server.hpp"

void Server::handleTopic(Client &client,
                         const std::vector<std::string> &args)
{
    if (!checkRegistered(client))
        return;

    if (!checkParams(client, args, 1, "TOPIC"))
        return;

    const std::string &channel = args[0];
    std::string topic;
    if (args.size() > 1)
        topic = args[1];

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

    Channel &channelObj = channelIt->second;

    if (topic.empty())
    {
        if (channelObj.getTopic().empty())
        {
            sendNumericReply(client.getFd(), "331", channel + " :No topic is set");
        }
        else
        {
            sendNumericReply(client.getFd(), "332", channel + " :" + channelObj.getTopic());
        }
        return;
    }

    if (channelObj.isTopicRestricted() && !channelObj.isOperator(client.getFd()))
    {
        sendNumericReply(client.getFd(), "482", channel + " :You're not channel operator");
        return;
    }

    channelObj.setTopic(topic);
    std::string reply = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost TOPIC " + channel + " :" + topic + "\r\n";

    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.isInChannel(channel))
            sendDataToClient(it->first, reply);
    }
}