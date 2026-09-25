#include "../../include/Server.hpp"

void Server::handlePart(Client &client,
                        const std::vector<std::string> &args)
{
    if (!checkRegistered(client))
        return;

    if (!checkParams(client, args, 1, "PART"))
        return;

    // Split the channel list by commas to handle multiple channels
    const std::string &channelList = args[0];

    size_t start = 0;

    while (start < channelList.size())
    {
        size_t comma = channelList.find(',', start);

        std::string channel;

        // Extract the channel name from the list
        if (comma == std::string::npos)
            channel = channelList.substr(start);
        else
            channel = channelList.substr(start, comma - start);

        // Check if the client is in the channel before allowing them to part
        if (!channel.empty())
        {
            if (!client.isInChannel(channel))
            {
                sendNumericReply(client.getFd(),
                                 "442",
                                 channel + " :You're not on that channel");
            }
            else
            {
                std::string reply = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost PART " + channel + "\r\n";

                // Notify all members of the channel
                for (std::map<int, Client>::iterator it = _clients.begin();
                     it != _clients.end();
                     ++it)
                {
                    if (it->second.isInChannel(channel))
                        sendDataToClient(it->first, reply);
                }

                // Remove the client from the channel
                client.partChannel(channel);

                // Find the channel in the server
                std::map<std::string, Channel>::iterator channelIt =
                    _channels.find(channel);

                if (channelIt != _channels.end())
                {
                    // Remove the client from the Channel
                    channelIt->second.removeClient(client.getFd());

                    // If nobody is left, destroy the channel
                    if (channelIt->second.getClientCount() == 0)
                        _channels.erase(channelIt);
                }

                std::cout << "Client " << client.getFd()
                          << " left channel: "
                          << channel << std::endl;
            }
        }

        if (comma == std::string::npos)
            break;

        start = comma + 1;
    }
}