#include "../../include/Server.hpp"

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

    const std::string &nickname = args[0];

    // Check if the nickname is valid according to IRC rules
    if (!isValidNickname(nickname))
    {
        sendNumericReply(client.getFd(), "432", nickname + " :Erroneous nickname");
        return;
    }

    // Check if the nickname is already in use by another client
    for (std::map<int, Client>::const_iterator it = _clients.begin();
         it != _clients.end();
         ++it)
    {
        if (it->first != client.getFd() && it->second.getNickname() == nickname)
        {
            sendNumericReply(client.getFd(),
                             "433",
                             ":Nickname is already in use");
            return;
        }
    }

    client.setNickname(nickname);

    std::cout << "Client " << client.getFd()
              << " set nickname to: "
              << client.getNickname() << std::endl;

    tryRegister(client);
}