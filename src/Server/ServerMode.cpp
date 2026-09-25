#include "../../include/Server.hpp"

void Server::handleMode(Client &client,
                        const std::vector<std::string> &args)
{
    if (!checkRegistered(client))
        return;

    if (!checkParams(client, args, 2, "MODE"))
        return;

    const std::string &channel = args[0];
    const std::string &mode = args[1];

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

    bool add = true;
    size_t argIndex = 2;
    std::string changes;

    for (size_t i = 0; i < mode.size(); ++i)
    {
        char c = mode[i];

        if (c == '+')
        {
            add = true;
            continue;
        }
        if (c == '-')
        {
            add = false;
            continue;
        }

        if (c == 'i')
        {
            channelIt->second.setInviteOnly(add);
            changes += (add ? "+i" : "-i");
            continue;
        }
        if (c == 't')
        {
            channelIt->second.setTopicRestricted(add);
            changes += (add ? "+t" : "-t");
            continue;
        }
        if (c == 'k')
        {
            if (add)
            {
                if (argIndex >= args.size())
                {
                    sendNumericReply(client.getFd(), "461", "MODE :Not enough parameters");
                    return;
                }
                channelIt->second.setPassword(args[argIndex++]);
                changes += "+k";
            }
            else
            {
                channelIt->second.setPassword("");
                changes += "-k";
            }
            continue;
        }
        if (c == 'l')
        {
            if (add)
            {
                if (argIndex >= args.size())
                {
                    sendNumericReply(client.getFd(), "461", "MODE :Not enough parameters");
                    return;
                }
                int limit = std::atoi(args[argIndex++].c_str());
                channelIt->second.setUserLimit(limit);
                changes += "+l";
            }
            else
            {
                channelIt->second.setUserLimit(0);
                changes += "-l";
            }
            continue;
        }
        if (c == 'o')
        {
            if (argIndex >= args.size())
            {
                sendNumericReply(client.getFd(), "461", "MODE :Not enough parameters");
                return;
            }
            const std::string &targetNick = args[argIndex++];
            std::map<int, Client>::iterator targetIt = _clients.end();
            for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
            {
                if (it->second.getNickname() == targetNick)
                {
                    targetIt = it;
                    break;
                }
            }
            if (targetIt == _clients.end() || !targetIt->second.isInChannel(channel))
            {
                sendNumericReply(client.getFd(), "441", targetNick + " " + channel + " :They aren't on that channel");
                return;
            }
            if (add)
            {
                channelIt->second.addOperator(targetIt->first);
                changes += "+o";
            }
            else
            {
                channelIt->second.removeOperator(targetIt->first);
                changes += "-o";
            }
            continue;
        }
    }

    if (!changes.empty())
    {
        std::string reply = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost MODE " + channel + " " + changes;
        if (argIndex <= args.size())
        {
            for (size_t j = 2; j < args.size(); ++j)
            {
                reply += " ";
                reply += args[j];
            }
        }
        reply += "\r\n";
        for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        {
            if (it->second.isInChannel(channel))
                sendDataToClient(it->first, reply);
        }
    }
}