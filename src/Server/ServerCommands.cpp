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

    sendDataToClient(client.getFd(),
                     "JOIN " + channel + "\r\n");

    sendNumericReply(client.getFd(),
                     "353",
                     client.getNickname() + " = " + channel + " :" + client.getNickname());

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

    client.partChannel(channel);

    std::cout << "Client " << client.getFd()
              << " left channel: "
              << channel << std::endl;

    sendDataToClient(client.getFd(),
                     "PART " + channel + "\r\n");
}

void Server::handlePrivMsg(Client &client,
                           const std::vector<std::string> &args)
{
    if (!client.isRegistered())
    {
        sendDataToClient(client.getFd(),
                         "451 :You have not registered\r\n");
        return;
    }

    if (args.size() < 2)
    {
        sendDataToClient(client.getFd(),
                         "461 PRIVMSG :Not enough parameters\r\n");
        return;
    }

    const std::string &target = args[0];
    const std::string &message = args[1];

    std::cout << "PRIVMSG from "
              << client.getNickname()
              << " to " << target
              << ": " << message << std::endl;

    /*
     * De momento solo devolvemos el mensaje al propio cliente.
     * Más adelante tendremos que buscar si target es:
     *   - otro nickname
     *   - un canal
     * y enviarlo a los clientes correspondientes.
     */

    std::string reply = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost PRIVMSG " + target + " :" + message + "\r\n";

    sendDataToClient(client.getFd(), reply);
}