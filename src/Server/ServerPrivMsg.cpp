#include "../../include/Server.hpp"

void Server::handlePrivMsg(Client &client,
                           const std::vector<std::string> &args)
{
    if (!checkParams(client, args, 2, "PRIVMSG"))
        return;

    if (!checkRegistered(client))
        return;

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