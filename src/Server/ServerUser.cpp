#include "../../include/Server.hpp"

void Server::handleUser(Client &client,
                        const std::vector<std::string> &args)
{
    if (client.isRegistered())
    {
        sendNumericReply(client.getFd(),
                         "462",
                         ":You may not reregister");
        return;
    }

    if (!client.isPassAccepted())
    {
        sendNumericReply(client.getFd(),
                         "464",
                         "You must authenticate first");
        return;
    }

    if (!checkParams(client, args, 4, "USER"))
        return;

    client.setUsername(args[0]);
    client.setRealname(args[3]);

    std::cout << "Client " << client.getFd()
              << " set username to: "
              << client.getUsername()
              << " and realname to: "
              << client.getRealname() << std::endl;

    tryRegister(client);
}