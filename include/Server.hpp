#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include <unistd.h>
#include <cstring>
#include <cctype>
#include <cstdlib>

#include <poll.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Client.hpp"
#include "Parser.hpp"
#include "Channel.hpp"
class Server
{
private:
    int _serverFd;
    sockaddr_in _serverAddr; // Server address structure
    bool _isRunning;         // Flag to indicate if the server is running
    Parser _parser;          // Parser object to handle incoming messages
    std::string _password;   // Password for client authentication

    std::vector<pollfd> _pollfds; // Vector to hold pollfd structures for polling
    std::map<int, Client> _clients;
    std::map<std::string, Channel> _channels;

    // FUNCTIONS FOR NETWORKING
    void acceptNewClient();
    void receiveDataFromClient(int clientFd);
    void sendDataToClient(int clientFd, const std::string &data);
    void queueDataToClient(int clientFd, const std::string &data);
    void processClientOutput(int clientFd);
    void setClientPollOut(int clientFd, bool enable);
    void removeClient(int clientFd);
    // function to send numeric replies to clients
    void sendNumericReply(int clientFd, const std::string &code, const std::string &message);

    // FUNCTIONS FOR COMMANDS
    void executeCommand(Client &client,
                        const std::string &command,
                        const std::vector<std::string> &args);
    void handlePass(Client &client, const std::vector<std::string> &args);
    void handleNick(Client &client, const std::vector<std::string> &args);
    void handleUser(Client &client, const std::vector<std::string> &args);
    void handleJoin(Client &client, const std::vector<std::string> &args);
    void handlePart(Client &client, const std::vector<std::string> &args);
    void handlePrivMsg(Client &client, const std::vector<std::string> &args);
    void handleInvite(Client &client, const std::vector<std::string> &args);
    void handleTopic(Client &client, const std::vector<std::string> &args);
    void handleMode(Client &client, const std::vector<std::string> &args);
    void handleKick(Client &client, const std::vector<std::string> &args);

    // helpers
    void tryRegister(Client &client);
    bool isValidNickname(const std::string &nickname) const;
    bool checkRegistered(Client &client);
    bool checkParams(Client &client,
                     const std::vector<std::string> &args,
                     size_t minimum,
                     const std::string &command);

public:
    Server();
    ~Server();
    Server(const Server &other);
    Server &operator=(const Server &other);

    // PUBLIC FUNCTIONS FOR NETWORKING
    void init(int port, const std::string &password);
    void run();
};

#endif