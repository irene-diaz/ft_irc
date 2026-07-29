#include <iostream>
#include <vector>
#include <string>
#include <poll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <cstdlib>
#include <cstdio>

class Server
{
private:
    int _serverFd;

    std::vector<pollfd> _pollfds;

public:
    Server();
    ~Server();

    int getServerFd() const { return _serverFd; };

    void init(int port, const std::string &password);
    void run();
};