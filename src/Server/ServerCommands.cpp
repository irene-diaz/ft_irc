#include "../../include/Server.hpp"
#include <cstdlib>

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

void Server::handlePass(Client &client, const std::vector<std::string> &args)
{
    if (args.empty())
    {
        sendNumericReply(client.getFd(),
                         "461",
                         "PASS :Not enough parameters");
        return;
    }

    if (args[0] != _password)
    {
        sendNumericReply(client.getFd(),
                         "464",
                         "Password incorrect");
        return;
    }

    client.setPassAccepted(true);

    std::cout << "Client " << client.getFd()
              << " authenticated successfully." << std::endl;

    tryRegister(client);
}

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

void Server::handleNick(Client &client, const std::vector<std::string> &args)
{
    if (!client.isPassAccepted())
    {
        sendNumericReply(client.getFd(),
                         "464",
                         "You must authenticate first");
        return;
    }

    if (args.empty())
    {
        sendNumericReply(client.getFd(),
                         "431",
                         ":No nickname given");
        return;
    }

    for (std::map<int, Client>::const_iterator it = _clients.begin();
         it != _clients.end();
         ++it)
    {
        if (it->first != client.getFd() && it->second.getNickname() == args[0])
        {
            sendNumericReply(client.getFd(),
                             "433",
                             ":Nickname is already in use");
            return;
        }
    }

    client.setNickname(args[0]);

    std::cout << "Client " << client.getFd()
              << " set nickname to: "
              << client.getNickname() << std::endl;

    tryRegister(client);
}

void Server::handleUser(Client &client,
                        const std::vector<std::string> &args)
{
    if (!client.isPassAccepted())
    {
        sendNumericReply(client.getFd(),
                         "464",
                         "You must authenticate first");
        return;
    }

    if (args.size() < 4)
    {
        sendNumericReply(client.getFd(),
                         "461",
                         "USER :Not enough parameters");
        return;
    }

    client.setUsername(args[0]);
    client.setRealname(args[3]);

    std::cout << "Client " << client.getFd()
              << " set username to: "
              << client.getUsername()
              << " and realname to: "
              << client.getRealname() << std::endl;

    tryRegister(client);
}

void Server::handleJoin(Client &client,
                        const std::vector<std::string> &args)
{
    if (!client.isRegistered())
    {
        sendNumericReply(client.getFd(),
                         "451",
                         ":You have not registered");
        return;
    }

    if (args.empty())
    {
        sendNumericReply(client.getFd(),
                         "461",
                         "JOIN :Not enough parameters");
        return;
    }

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
            // Don't join the same channel twice
            if (client.isInChannel(channel))
            {
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

void Server::handlePart(Client &client,
                        const std::vector<std::string> &args)
{
    if (!client.isRegistered())
    {
        sendNumericReply(client.getFd(),
                         "451",
                         ":You have not registered");
        return;
    }

    if (args.empty())
    {
        sendNumericReply(client.getFd(),
                         "461",
                         "PART :Not enough parameters");
        return;
    }

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

void Server::handlePrivMsg(Client &client,
                           const std::vector<std::string> &args)
{
    if (args.empty())
    {
        sendNumericReply(client.getFd(),
                         "461",
                         "PRIVMSG :Not enough parameters");
        return;
    }

    if (!client.isRegistered())
    {
        sendNumericReply(client.getFd(),
                         "451",
                         ":You have not registered");
        return;
    }

    if (args.size() < 2)
    {
        sendNumericReply(client.getFd(),
                         "412",
                         "PRIVMSG :No text to send");
        return;
    }

    const std::string &target = args[0]; // username or channel
    const std::string &message = args[1];

    // Check if the target is empty
    if (target.empty())
    {
        sendNumericReply(client.getFd(),
                         "411",
                         "No recipient");
        return;
    }

    std::cout << "PRIVMSG from "
              << client.getNickname()
              << " to " << target
              << ": " << message << std::endl;

    std::string reply = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost PRIVMSG " + target + " :" + message + "\r\n";

    // channel message
    if (target[0] == '#')
    {
        // Check if the client is in the channel before broadcasting
        if (!client.isInChannel(target))
        {
            sendNumericReply(client.getFd(),
                             "404",
                             target + " :Cannot send to channel");
            return;
        }

        for (std::map<int, Client>::iterator it = _clients.begin();
             it != _clients.end();
             ++it)
        {
            if (it->first == client.getFd())
                continue;

            if (it->second.isInChannel(target))
                sendDataToClient(it->first, reply);
        }
    }
    // user message
    else
    {
        bool found = false; // flag to check if the target user was found

        for (std::map<int, Client>::iterator it = _clients.begin();
             it != _clients.end();
             ++it)
        {
            if (it->first == client.getFd())
                continue;

            if (it->second.getNickname() == target)
            {
                sendDataToClient(it->first, reply);
                found = true;
                break; // stop searching after sending the message to the target user
            }
        }

        if (!found)
        {
            sendNumericReply(client.getFd(),
                             "401",
                             target + " :No such nickname");
        }
    }
}

void Server::handleKick(Client &client,
                        const std::vector<std::string> &args)
{
    if (!client.isRegistered())
    {
        sendNumericReply(client.getFd(),
                         "451",
                         ":You have not registered");
        return;
    }

    if (args.size() < 2)
    {
        sendNumericReply(client.getFd(),
                         "461",
                         "KICK :Not enough parameters");
        return;
    }

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

void Server::handleInvite(Client &client,
                          const std::vector<std::string> &args)
{
    if (!client.isRegistered())
    {
        sendNumericReply(client.getFd(), "451", ":You have not registered");
        return;
    }

    if (args.size() < 2)
    {
        sendNumericReply(client.getFd(), "461", "INVITE :Not enough parameters");
        return;
    }

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

void Server::handleTopic(Client &client,
                          const std::vector<std::string> &args)
{
    if (!client.isRegistered())
    {
        sendNumericReply(client.getFd(), "451", ":You have not registered");
        return;
    }

    if (args.empty())
    {
        sendNumericReply(client.getFd(), "461", "TOPIC :Not enough parameters");
        return;
    }

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

void Server::handleMode(Client &client,
                        const std::vector<std::string> &args)
{
    if (!client.isRegistered())
    {
        sendNumericReply(client.getFd(), "451", ":You have not registered");
        return;
    }

    if (args.size() < 2)
    {
        sendNumericReply(client.getFd(), "461", "MODE :Not enough parameters");
        return;
    }

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
