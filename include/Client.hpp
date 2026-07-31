#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
    int _fd;
    std::string _recvBuffer;

    bool _passAccepted;
    /*std::string _nickname;
    std::string _username;
    std::string _realname;*/

public:
    Client();
    Client(int fd);
    ~Client();
    Client(const Client &other);
    Client &operator=(const Client &other);
    void appendToBuffer(const std::string &data);
    bool hasCompleteLine() const;
    std::string extractLine();
    const std::string &getRecvBuffer() const;
    bool isPassAccepted() const;
    void setPassAccepted(bool accepted);
};

#endif