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
    bool _isRunning;         // Flag to indicate if the server is running

    std::vector<pollfd> _pollfds; // Vector to hold pollfd structures for polling
    std::map<int, Client> _clients;
    void acceptNewClient();
    void receiveDataFromClient(int clientFd);
    void sendDataToClient(int clientFd, const std::string &data);
    void removeClient(int clientFd);

public:
    Server();
    ~Server();
    Server(const Server &other);
    Server &operator=(const Server &other);

    void init(int port, const std::string &password);
    void run();
};

#endif