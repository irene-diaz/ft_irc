#include "../include/Server.hpp"

// default value for _serverFd is -1, which indicates that the server is not yet initialized
Server::Server() : _serverFd(-1)
{
}

Server::~Server()
{
    if (_serverFd != -1)
        close(_serverFd);
}

// Initialize the server socket, bind it to the specified port, and start listening for incoming connections
void Server::init(int port, const std::string &password)
{
    (void)port;
    (void)password;

    // Create a socket(domain, type, protocol), return a file descriptor for the new socket
    /*AF_INET: IPv4 Internet protocols
    SOCK_STREAM: Provides sequenced, reliable, two-way, connection-based byte streams.
    0: Use the default protocol for the specified socket type*/
    _serverFd = socket(AF_INET, SOCK_STREAM, 0);

    if (_serverFd == -1)
    {
        throw std::runtime_error("Failed to create socket");
    }
    int opt = 1;

    // Set socket options to allow reuse of the address and port
    /*SO_REUSEADDR: Allows the socket to be bound to an address that is already in use*/
    if (setsockopt(_serverFd,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &opt,
                   sizeof(opt)) == -1)
    {
        throw std::runtime_error("Failed to set socket options");
    }

    // Initialize the server address structure with 0s, for cleaning the structure before using it
    memset(&_serverAddr, 0, sizeof(_serverAddr));

    // sockaddr_in es una estructura que represnta una direccion web
    //  AF_INET: IPv4 Internet protocols
    _serverAddr.sin_family = AF_INET;
    // Convert the port number from host byte order to network byte order and save
    // htons=host to network short, converts a short from pc format to network format
    _serverAddr.sin_port = htons(port);
    // INADDR_ANY: IP: Bind the socket to all available network interfaces
    _serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the specified address and port
    if (bind(_serverFd,
             reinterpret_cast<sockaddr *>(&_serverAddr),
             sizeof(_serverAddr)) == -1)
    {
        throw std::runtime_error("Failed to bind socket");
    }

    // Start listening for incoming connections on the socket
    if (listen(_serverFd, SOMAXCONN) == -1)
    {
        throw std::runtime_error("Failed to listen");
    }
    // Add the server socket to the pollfds vector
    pollfd serverPoll;

    serverPoll.fd = _serverFd;
    serverPoll.events = POLLIN;
    serverPoll.revents = 0;

    _pollfds.push_back(serverPoll);
}

void Server::acceptNewClient(int _serverFd, std::vector<pollfd> &pollfds)
{
    // Accept an incoming connection and return a new file descriptor for the accepted socket
    /*accept(fd, addr, addrlen)*/
    int clientFd = accept(_serverFd, NULL, NULL);
    if (clientFd == -1)
    {
        throw std::runtime_error("Failed to accept new client");
    }

    // Add the new client socket to the pollfds vector
    pollfd pfd;
    pfd.fd = clientFd;
    pfd.events = POLLIN; // Monitor for incoming data
    pfd.revents = 0;     // Initialize revents to 0
    pollfds.push_back(pfd);

    std::cout << "New client connected: " << clientFd << std::endl;
}

void Server::receiveDataFromClient(int clientFd)
{
    char buffer[1024];
    // Receive data from the client socket
    /*recv(fd, buf, len, flags)*/
    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead == -1)
    {
        throw std::runtime_error("Failed to receive data from client");
    }
    else if (bytesRead == 0)
    {
        // Client disconnected
        std::cout << "Client disconnected: " << clientFd << std::endl;
        close(clientFd);
    }
    else
    {
        buffer[bytesRead] = '\0'; // Null-terminate the received data
        std::cout << "Received data from client " << clientFd << ": " << buffer << std::endl;
    }
}
void Server::run()
{
    while (true)
    {
        int ready = poll(&_pollfds[0], _pollfds.size(), -1);

        if (ready == -1)
            throw std::runtime_error("poll failed");

        for (size_t i = 0; i < _pollfds.size(); i++)
        {
            pollfd &current = _pollfds[i];

            if (!(current.revents & POLLIN))
                continue;

            if (current.fd == _serverFd)
                acceptNewClient(_serverFd, _pollfds);
            else
                receiveDataFromClient(current.fd);
        }
    }
}