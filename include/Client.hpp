#ifndef CLIENT_HPP
#define CLIENT_HPP

class Client
{
private:
    int _fd;
    /*std::string _nickname;
    std::string _username;
    std::string _realname;*/

public:
    Client();
    Client(int fd);
    ~Client();
    void setFd(int fd);
    int getFd() const;
};

#endif