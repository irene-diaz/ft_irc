#include "../include/Client.hpp"

Client::Client(int fd)
    : _fd(fd)
{
}

Client::~Client()
{
}

Client::Client()
    : _fd(-1)
{
}

Client::Client(const Client &other)
    : _fd(other._fd), _recvBuffer(other._recvBuffer)
{
}

Client &Client::operator=(const Client &other)
{
    if (this != &other)
    {
        _fd = other._fd;
        _recvBuffer = other._recvBuffer;
    }
    return *this;
}

void Client::appendToBuffer(const std::string &data)
{
    _recvBuffer += data;
}

bool Client::hasCompleteLine() const
{
    return _recvBuffer.find("\r\n") != std::string::npos;
}

std::string Client::extractLine()
{
    size_t pos = _recvBuffer.find("\r\n");
    if (pos == std::string::npos)
        return "";

    std::string line = _recvBuffer.substr(0, pos);
    _recvBuffer.erase(0, pos + 2); // Remove the extracted lines from the buffer
    return line;
}

const std::string &Client::getRecvBuffer() const
{
    return _recvBuffer;
}