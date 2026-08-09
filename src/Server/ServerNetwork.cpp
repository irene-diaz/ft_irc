#include "../../include/Server.hpp"
#include <fcntl.h>
#include <cerrno>

static void setNonBlockingFd(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error("Failed to get socket flags");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error("Failed to set socket non-blocking");
}

void Server::sendNumericReply(int clientFd, const std::string &code, const std::string &message)
{
    std::string reply = ":ircserv " + code + " " + message + "\r\n";
    queueDataToClient(clientFd, reply);
}

void Server::sendDataToClient(int clientFd, const std::string &data)
{
    queueDataToClient(clientFd, data);
}

// Initialize the server socket, bind it to the specified port, and start listening for incoming connections
void Server::init(int port, const std::string &password)
{
    _password = password; // Store the password for client authentication

    // Create a socket(domain, type, protocol), return a file descriptor for the new socket
    /*AF_INET: IPv4 Internet protocols
    SOCK_STREAM: Provides sequenced, reliable, two-way, connection-based byte streams. (TCP)
    0: Use the default protocol for the specified socket type*/
    _serverFd = socket(AF_INET, SOCK_STREAM, 0);

    if (_serverFd == -1)
    {
        throw std::runtime_error("Failed to create socket");
    }

    setNonBlockingFd(_serverFd);
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
void Server::acceptNewClient()
{
    // Accept an incoming connection and return a new file descriptor for the accepted socket
    /*accept(fd, addr, addrlen)*/
    int clientFd = accept(_serverFd, NULL, NULL);
    if (clientFd == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        throw std::runtime_error("Failed to accept new client");
    }

    setNonBlockingFd(clientFd);

    // Create a new Client object for the accepted client socket and add it to the _clients map
    Client client(clientFd);
    _clients.insert(std::make_pair(clientFd, client));
    // Add the new client socket to the pollfds vector
    pollfd pfd;
    pfd.fd = clientFd;
    pfd.events = POLLIN; // Monitor for incoming data
    pfd.revents = 0;     // Initialize revents to 0

    _pollfds.push_back(pfd); // Add the new client socket to the pollfds vector for monitoring incoming data

    std::cout << "New client connected: " << clientFd << std::endl;
}

// Receive data from a client socket and handle it
void Server::receiveDataFromClient(int clientFd)
{
    // EXTRA: Print a message indicating that we are entering the receiveDataFromClient() function
    // std::cout << "Entrando en receiveDataFromClient()" << std::endl;
    char buffer[1024];
    // Receive data from the client socket
    /*recv(fd, buf, len, flags)*/
    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        throw std::runtime_error("Failed to receive data from client");
    }
    else if (bytesRead == 0)
    {
        // Client disconnected
        std::cout << "Client disconnected: " << clientFd << std::endl;
        removeClient(clientFd);
        return;
    }
    else
    {
        buffer[bytesRead] = '\0'; // Null-terminate the received data
        std::map<int, Client>::iterator it = _clients.find(clientFd);

        if (it == _clients.end())
            return;

        Client &client = it->second;
        client.appendToBuffer(buffer); // Append the received data to the client's buffer
        while (client.hasCompleteLine())
        {
            std::string line = client.extractLine();
            std::vector<std::string> args;

            std::string command = _parser.parseCommand(line, args);

            executeCommand(client, command, args);
            // DEBUG: Print the complete line received from the client for debugging purposes
            //  std::cout << "Complete line from client " << clientFd << ": " << line << std::endl;
        }
    }
}
// Run the server, continuously polling for incoming connections and data from clients
void Server::run()
{
    _isRunning = true;

    while (_isRunning)
    {
        std::vector<pollfd> pollfds = _pollfds;

        int ready = poll(&pollfds[0], pollfds.size(), -1);

        if (ready == -1)
            throw std::runtime_error("poll failed");

        for (size_t i = 0; i < pollfds.size(); ++i)
        {
            int fd = pollfds[i].fd;

            if (fd == _serverFd)
            {
                if (pollfds[i].revents & POLLIN)
                    acceptNewClient();
                continue;
            }

            if (pollfds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
            {
                removeClient(fd);
                continue;
            }

            if (pollfds[i].revents & POLLIN)
                receiveDataFromClient(fd);

            if (pollfds[i].revents & POLLOUT)
                processClientOutput(fd);
        }
    }
}

void Server::removeClient(int clientFd)
{
    std::map<int, Client>::iterator clientIt = _clients.find(clientFd);
    if (clientIt == _clients.end())
        return;

    Client &client = clientIt->second;
    std::string nickname = client.getNickname();
    std::string username = client.getUsername();

    // Get list of channels before erasing
    std::vector<std::string> channels;
    for (size_t i = 0; i < client.isInChannel("") ? 1 : 0; ++i)
        ;

    // Extract channel list manually
    std::vector<std::string> channelsToLeave;
    std::map<std::string, Channel>::iterator chanIt = _channels.begin();
    while (chanIt != _channels.end())
    {
        if (chanIt->second.hasClient(clientFd))
            channelsToLeave.push_back(chanIt->first);
        ++chanIt;
    }

    // Notify all channels and remove client from them
    for (size_t i = 0; i < channelsToLeave.size(); ++i)
    {
        const std::string &channel = channelsToLeave[i];
        std::string quitMsg = ":" + nickname + "!" + username + "@localhost QUIT :Client disconnected\r\n";

        std::map<std::string, Channel>::iterator it = _channels.find(channel);
        if (it != _channels.end())
        {
            for (std::map<int, Client>::iterator otherclient = _clients.begin();
                 otherclient != _clients.end();
                 ++otherclient)
            {
                if (otherclient->first != clientFd && otherclient->second.isInChannel(channel))
                    queueDataToClient(otherclient->first, quitMsg);
            }
            it->second.removeClient(clientFd);
            if (it->second.getClientCount() == 0)
                _channels.erase(it);
        }
    }

    close(clientFd);
    _clients.erase(clientIt);

    for (std::vector<pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end(); ++it)
    {
        if (it->fd == clientFd)
        {
            _pollfds.erase(it);
            break;
        }
    }
    std::cout << "Client removed: " << clientFd << std::endl;
}

void Server::setClientPollOut(int clientFd, bool enable)
{
    for (std::vector<pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end(); ++it)
    {
        if (it->fd == clientFd)
        {
            if (enable)
                it->events |= POLLOUT;
            else
                it->events &= ~POLLOUT;
            return;
        }
    }
}

void Server::queueDataToClient(int clientFd, const std::string &data)
{
    std::map<int, Client>::iterator it = _clients.find(clientFd);
    if (it == _clients.end())
        return;

    Client &client = it->second;
    client.appendToSendBuffer(data);
    setClientPollOut(clientFd, true);
}

void Server::processClientOutput(int clientFd)
{
    std::map<int, Client>::iterator it = _clients.find(clientFd);
    if (it == _clients.end())
        return;

    Client &client = it->second;
    if (!client.hasPendingOutput())
    {
        setClientPollOut(clientFd, false);
        return;
    }

    const std::string &buffer = client.getSendBuffer();
    ssize_t bytesSent = send(clientFd, buffer.c_str(), buffer.size(), 0);
    if (bytesSent > 0)
    {
        client.consumeSendBuffer(static_cast<size_t>(bytesSent));
        if (!client.hasPendingOutput())
            setClientPollOut(clientFd, false);
        return;
    }

    if (bytesSent == 0)
    {
        // Peer closed the connection
        removeClient(clientFd);
        return;
    }

    if (bytesSent == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        removeClient(clientFd);
    }
}
