#include "../../include/Server.hpp"
#include <ctime>

static std::string toLowerString(const std::string &value)
{
    std::string result = value;
    for (size_t i = 0; i < result.size(); ++i)
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
    return result;
}

static std::string trimWhitespace(const std::string &value)
{
    size_t start = 0;
    while (start < value.size() && (value[start] == ' ' || value[start] == '\t' || value[start] == '\r' || value[start] == '\n'))
        ++start;

    size_t end = value.size();
    while (end > start && (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r' || value[end - 1] == '\n'))
        --end;

    return value.substr(start, end - start);
}

bool Server::isBotTarget(const std::string &target) const
{
    if (target.empty())
        return false;

    std::string lower = toLowerString(target);
    return lower == "bot" || lower == "ircbot" || lower == "botty";
}

void Server::handleBotCommand(Client &client,
                             const std::string &target,
                             const std::string &message)
{
    std::string command = trimWhitespace(message);
    if (!command.empty() && command[0] == '!')
        command = command.substr(1);

    std::string lower = toLowerString(command);
    std::string response;

    if (lower == "help")
        response = "Available commands: !help, !time, !ping, !whoami, !hello";
    else if (lower == "time")
    {
        time_t now = time(NULL);
        std::string timestamp = ctime(&now);
        if (!timestamp.empty() && timestamp[timestamp.size() - 1] == '\n')
            timestamp.erase(timestamp.size() - 1);
        response = "Current server time: " + timestamp;
    }
    else if (lower == "ping")
        response = "PONG! I am listening.";
    else if (lower == "whoami")
        response = "You are " + client.getNickname() + ".";
    else if (lower == "hello")
        response = "Hello " + client.getNickname() + "! I am IRCBot.";
    else
        response = "Unknown command. Try !help.";

    std::string reply = ":IRCBot!bot@localhost PRIVMSG " + target + " :" + response + "\r\n";

    if (target[0] == '#')
    {
        for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        {
            if (it->second.isInChannel(target))
                sendDataToClient(it->first, reply);
        }
        return;
    }

    sendDataToClient(client.getFd(), reply);
}

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

    const std::string &target = args[0];
    const std::string &message = args[1];

    if (target.empty())
    {
        sendNumericReply(client.getFd(),
                         "411",
                         "No recipient");
        return;
    }

    if (isBotTarget(target) || (!target.empty() && target[0] == '#' && message.size() > 1 && message[0] == '!'))
    {
        handleBotCommand(client, target, message);
        return;
    }

    std::cout << "PRIVMSG from "
              << client.getNickname()
              << " to " << target
              << ": " << message << std::endl;

    std::string reply = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost PRIVMSG " + target + " :" + message + "\r\n";

    if (target[0] == '#')
    {
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
    else
    {
        bool found = false;

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
                break;
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