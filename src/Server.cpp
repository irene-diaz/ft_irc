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

    // Initialize the server address structure with 0s
    memset(&_serverAddr, 0, sizeof(_serverAddr));

    // AF_INET: IPv4 Internet protocols
    _serverAddr.sin_family = AF_INET;
    // Convert the port number from host byte order to network byte order
    _serverAddr.sin_port = htons(port);
    // INADDR_ANY: Bind the socket to all available network interfaces
    _serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(_serverFd,
             reinterpret_cast<sockaddr *>(&_serverAddr),
             sizeof(_serverAddr)) == -1)
    {
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(_serverFd, SOMAXCONN) == -1)
    {
        throw std::runtime_error("Failed to listen");
    }
}