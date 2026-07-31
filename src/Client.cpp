#include "../include/Client.hpp"

Client::Client(int fd)
    : _fd(fd), _passAccepted(false)
{
}

Client::~Client()
{
}

Client::Client()
    : _fd(-1), _passAccepted(false)
{
}

Client::Client(const Client &other)
    : _fd(other._fd), _recvBuffer(other._recvBuffer), _passAccepted(other._passAccepted)
{
}

Client &Client::operator=(const Client &other)
{
    if (this != &other)
    {
        _fd = other._fd;
        _recvBuffer = other._recvBuffer;
        _passAccepted = other._passAccepted;
    }
    return *this;
}

// functions to manage the receive buffer
void Client::appendToBuffer(const std::string &data)
{
    _recvBuffer += data;
}

// Check if the receive buffer contains a complete line (terminated by '\n')
bool Client::hasCompleteLine() const
{
    return _recvBuffer.find("\n") != std::string::npos;
}

// Extract a complete line from the receive buffer, if available
std::string Client::extractLine()
{
    size_t pos = _recvBuffer.find("\n");
    if (pos == std::string::npos)
        return "";

    std::string line = _recvBuffer.substr(0, pos);
    // Remove the trailing carriage return if present
    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);

    _recvBuffer.erase(0, pos + 1); // Remove the extracted lines from the buffer
    return line;
}

// Get the current receive buffer
const std::string &Client::getRecvBuffer() const
{
    return _recvBuffer;
}

bool Client::isPassAccepted() const
{
    return _passAccepted;
}

void Client::setPassAccepted(bool accepted)
{
    _passAccepted = accepted;
}