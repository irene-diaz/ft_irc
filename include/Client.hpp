#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>

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

    std::vector<std::string> _channels;

public:
    Client();
    Client(int fd);
    ~Client();
    Client(const Client &other);
    Client &operator=(const Client &other);

    // Connection / buffer
    int getFd() const;
    void appendToBuffer(const std::string &data);
    bool hasCompleteLine() const;
    std::string extractLine();
    const std::string &getRecvBuffer() const;

    // Authentication / registration
    bool isPassAccepted() const;
    void setPassAccepted(bool accepted);

    bool isRegistered() const;
    void setRegistered(bool registered);

    // User information
    const std::string &getNickname() const;
    void setNickname(const std::string &nickname);

    const std::string &getUsername() const;
    void setUsername(const std::string &username);

    const std::string &getRealname() const;
    void setRealname(const std::string &realname);

    // Channels
    void joinChannel(const std::string &channel);
    void partChannel(const std::string &channel);
    bool isInChannel(const std::string &channel) const;
};

#endif