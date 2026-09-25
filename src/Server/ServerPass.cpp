#include "../../include/Server.hpp"

void Server::handlePass(Client &client, const std::vector<std::string> &args)
{
    if (client.isRegistered())
    {
        sendNumericReply(client.getFd(),
                         "462",
                         ":You may not reregister");
        return;
    }

    if (!checkParams(client, args, 1, "PASS"))
        return;

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