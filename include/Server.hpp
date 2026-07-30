#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include <unistd.h>
#include <cstring>

#include <poll.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Client.hpp"
class Server
{
private:
    int _serverFd;
    sockaddr_in _serverAddr; // Server address structure

    std::vector<pollfd> _pollfds; // Vector to hold pollfd structures for polling
    std::map<int, Client> _clients;
    void acceptNewClient(int _serverFd, std::vector<pollfd> &pollfds);
    void receiveDataFromClient(int clientFd);

public:
    Server();
    ~Server();
    Server(const Server &other);
    Server &operator=(const Server &other);

    int getServerFd() const { return _serverFd; };

    void init(int port, const std::string &password);
    void run();
};

#endif