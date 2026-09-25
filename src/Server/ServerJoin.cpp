#include "../../include/Server.hpp"

void Server::handleJoin(Client &client,
                        const std::vector<std::string> &args)
{
    if (!checkRegistered(client))
        return;

    if (!checkParams(client, args, 1, "JOIN"))
        return;

    const std::string &channelList = args[0];

    size_t start = 0;

    while (start < channelList.size())
    {
        size_t comma = channelList.find(',', start);

        std::string channel;

        if (comma == std::string::npos)
            channel = channelList.substr(start);
        else
            channel = channelList.substr(start, comma - start);

        // Ignore empty channel names
        if (!channel.empty())
        {
            if (channel.size() < 2 || channel[0] != '#')
            {
                sendNumericReply(client.getFd(),
                                 "403",
                                 channel + " :No such channel");

                if (comma == std::string::npos)
                    break;

                start = comma + 1;
                continue;
            }
            // Don't join the same channel twice
            if (client.isInChannel(channel))
            {
                sendNumericReply(client.getFd(),
                                 "443",
                                 channel + " :is already on channel");
                if (comma == std::string::npos)
                    break;

                start = comma + 1;
                continue;
            }
            std::string key;
            if (args.size() > 1)
            {
                size_t keyComma = channelList.find(',', start);
                size_t keyEnd = (keyComma == std::string::npos) ? std::string::npos : keyComma - start;
                if (keyComma == std::string::npos)
                    key = args[1].substr(start);
                else
                    key = args[1].substr(start, keyEnd);
            }

            if (_channels.find(channel) == _channels.end())
            {
                // If the channel doesn't exist, create it
                _channels[channel] = Channel(channel);

                // Set the joining client as an operator for the new channel
                _channels[channel].addOperator(client.getFd());

                // If a key was provided for the new channel, set it
                if (!key.empty())
                    _channels[channel].setPassword(key);
            }

            Channel &channelObj = _channels[channel];

            if (channelObj.isInviteOnly() && !channelObj.isInvitedClient(client.getFd()) && !channelObj.isOperator(client.getFd()))
            {
                sendNumericReply(client.getFd(), "473", channel + " :Cannot join channel (+i)");
                if (comma == std::string::npos)
                    break;
                start = comma + 1;
                continue;
            }

            if (!channelObj.getPassword().empty())
            {
                if (key.empty() || key != channelObj.getPassword())
                {
                    sendNumericReply(client.getFd(), "475", channel + " :Cannot join channel (+k)");
                    if (comma == std::string::npos)
                        break;
                    start = comma + 1;
                    continue;
                }
            }

            if (channelObj.getUserLimit() > 0 && static_cast<int>(channelObj.getClientCount()) >= channelObj.getUserLimit())
            {
                sendNumericReply(client.getFd(), "471", channel + " :Cannot join channel (+l)");
                if (comma == std::string::npos)
                    break;
                start = comma + 1;
                continue;
            }

            // Add the client to the channel's client list and mark them as joined
            channelObj.addClient(client.getFd());
            channelObj.removeInvitedClient(client.getFd());

            // Add the channel to the client's list of joined channels
            client.joinChannel(channel);

            std::cout << "Client " << client.getFd()
                      << " joined channel: "
                      << channel << std::endl;

            std::string reply = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost JOIN " + channel + "\r\n";

            // Broadcast the join message to all clients in the channel
            for (std::map<int, Client>::iterator it = _clients.begin();
                 it != _clients.end();
                 ++it)
            {
                if (it->second.isInChannel(channel))
                    sendDataToClient(it->first, reply);
            }

            std::string names; // Prepare a list of names in the channel

            for (std::map<int, Client>::iterator it = _clients.begin();
                 it != _clients.end();
                 ++it)
            {
                if (it->second.isInChannel(channel))
                {
                    if (!names.empty())
                        names += " ";

                    if (channelObj.isOperator(it->second.getFd()))
                        names += "@";

                    names += it->second.getNickname();
                }
            }
            // send a list of names in the channel
            sendNumericReply(client.getFd(),
                             "353",
                             client.getNickname() + " = " + channel + " :" + names);

            // send the end of names list message
            sendNumericReply(client.getFd(),
                             "366",
                             client.getNickname() + " " + channel + " :End of /NAMES list");
        }

        if (comma == std::string::npos)
            break;
        start = comma + 1;
    }
}