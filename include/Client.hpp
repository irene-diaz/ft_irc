#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
    int _fd;
    std::string _recvBuffer;

    bool _passAccepted;
    bool _isRegistered;
    std::string _nickname;
    std::string _username;
    std::string _realname;

public:
    Client();
    Client(int fd);
    ~Client();
    Client(const Client &other);
    int getFd() const;
    Client &operator=(const Client &other);
    void appendToBuffer(const std::string &data);
    bool hasCompleteLine() const;
    std::string extractLine();
    const std::string &getRecvBuffer() const;

    bool isPassAccepted() const;
    void setPassAccepted(bool accepted);

    bool isRegistered() const;
    void setRegistered(bool registered);

    const std::string &getNickname() const;
    void setNickname(const std::string &nickname);

    const std::string &getUsername() const;
    void setUsername(const std::string &username);

    const std::string &getRealname() const;
    void setRealname(const std::string &realname);
};

#endif