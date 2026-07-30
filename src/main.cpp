#include "../include/Server.hpp"
#include <cstdlib>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return (1);
    }

    try
    {
        Server server;
        int port = atoi(argv[1]);

        if (port <= 0 || port > 65535)
        {
            std::cerr << "Invalid port" << std::endl;
            return (1);
        }
        server.init(port, argv[2]);
        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return (1);
    }

    return (0);
}