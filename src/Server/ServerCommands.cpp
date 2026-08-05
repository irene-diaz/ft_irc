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

    const std::string &channel = args[0];

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
    // send a list of names in the channel (for simplicity, we only send the joining client's nickname)
    sendNumericReply(client.getFd(),
                     "353",
                     client.getNickname() + " = " + channel + " :" + names);

    // send the end of names list message
    sendNumericReply(client.getFd(),
                     "366",
                     client.getNickname() + " " + channel + " :End of /NAMES list");
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

    const std::string &channel = args[0];

    if (!client.isInChannel(channel))
    {
        sendNumericReply(client.getFd(),
                         "442",
                         channel + " :You're not on that channel");
        return;
    }

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

    std::cout << "Client " << client.getFd()
              << " left channel: "
              << channel << std::endl;
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