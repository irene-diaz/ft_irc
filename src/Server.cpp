#include "../include/Server.hpp"

// default value for _serverFd is -1, which indicates that the server is not yet initialized
Server::Server() : _serverFd(-1)
{
}

Server::~Server()
{
    // Close the server socket if it was initialized
    if (_serverFd != -1)
        close(_serverFd);
}

Server::Server(const Server &other)
    : _serverFd(other._serverFd), _serverAddr(other._serverAddr), _pollfds(other._pollfds), _clients(other._clients)
{
}

Server &Server::operator=(const Server &other)
{
    if (this != &other)
    {
        _serverFd = other._serverFd;
        _serverAddr = other._serverAddr;
        _pollfds = other._pollfds;
        _clients = other._clients;
    }
    return *this;
}

// Initialize the server socket, bind it to the specified port, and start listening for incoming connections
void Server::init(int port, const std::string &password)
{
    (void)password;

    // Create a socket(domain, type, protocol), return a file descriptor for the new socket
    /*AF_INET: IPv4 Internet protocols
    SOCK_STREAM: Provides sequenced, reliable, two-way, connection-based byte streams. (TCP)
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

    // sockaddr_in is a structure that contains an internet address. It is used with IPv4 addresses.
    //  AF_INET: IPv4 Internet protocols
    _serverAddr.sin_family = AF_INET;
    // Convert the port number from host byte order to network byte order and save
    // htons=host to network short, converts a short from pc format to network format
    _serverAddr.sin_port = htons(port);
    // INADDR_ANY: IP: Bind the socket to all available network interfaces
    _serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the specified address and port and return 0 on success, or -1 on error
    // the casting is necessary because bind() expects a sockaddr pointer, but we have a sockaddr_in structure (use for IPv4 addresses)
    if (bind(_serverFd,
             reinterpret_cast<sockaddr *>(&_serverAddr),
             sizeof(_serverAddr)) == -1)
    {
        throw std::runtime_error("Failed to bind socket");
    }

    // Start listening for incoming connections on the socket and return 0 on success, or -1 on error
    if (listen(_serverFd, SOMAXCONN) == -1)
    {
        throw std::runtime_error("Failed to listen");
    }
    // Add the server socket to the pollfds vector
    pollfd serverPoll;

    serverPoll.fd = _serverFd;
    serverPoll.events = POLLIN;
    serverPoll.revents = 0;

    _pollfds.push_back(serverPoll); // Add the server socket to the pollfds vector for monitoring incoming connections
}

// Accept a new client connection and add it to the pollfds vector
void Server::acceptNewClient(int _serverFd, std::vector<pollfd> &pollfds)
{
    // Accept an incoming connection and return a new file descriptor for the accepted socket
    /*accept(fd, addr, addrlen)*/
    int clientFd = accept(_serverFd, NULL, NULL);
    if (clientFd == -1)
    {
        throw std::runtime_error("Failed to accept new client");
    }

    // Create a new Client object for the accepted client socket and add it to the _clients map
    Client client(clientFd);
    _clients.insert(std::make_pair(clientFd, client));
    // Add the new client socket to the pollfds vector
    pollfd pfd;
    pfd.fd = clientFd;
    pfd.events = POLLIN;    // Monitor for incoming data
    pfd.revents = 0;        // Initialize revents to 0
    pollfds.push_back(pfd); // Add the new client socket to the pollfds vector for monitoring incoming data

    std::cout << "New client connected: " << clientFd << std::endl;
}

// Receive data from a client socket and handle it
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
        Client &client = _clients[clientFd];
        client.appendToBuffer(buffer); // Append the received data to the client's buffer
        while (client.hasCompleteLine())
        {
            std::string line = client.extractLine();
            std::cout << "Complete line from client " << clientFd << ": " << line << std::endl;
            // Here you can process the complete line received from the client
        }
    }
}
// Run the server, continuously polling for incoming connections and data from clients
void Server::run()
{
    while (true)
    {
        // Poll the file descriptors in the pollfds vector for events (incoming connections or data)
        int ready = poll(&_pollfds[0], _pollfds.size(), -1);

        if (ready == -1)
            throw std::runtime_error("poll failed");

        // Iterate through the pollfds vector to check which file descriptors have events
        for (size_t i = 0; i < _pollfds.size(); i++)
        {
            // Reference to the current pollfd structure
            pollfd &current = _pollfds[i];

            // Check if the current file descriptor has incoming data (POLLIN event)
            if (!(current.revents & POLLIN))
                continue;

            // If the current file descriptor is the server socket, accept a new client connection
            if (current.fd == _serverFd)
                acceptNewClient(_serverFd, _pollfds);
            // If the current file descriptor is a client socket, receive data from the client
            else
                receiveDataFromClient(current.fd);
        }
    }
}