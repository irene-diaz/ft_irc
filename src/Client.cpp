#include "../include/Client.hpp"

Client::Client(int fd)
    : _fd(fd), _passAccepted(false), _isRegistered(false)
{
}

Client::~Client()
{
}

Client::Client()
    : _fd(-1), _passAccepted(false), _isRegistered(false)
{
}

Client::Client(const Client &other)
    : _fd(other._fd), _recvBuffer(other._recvBuffer), _passAccepted(other._passAccepted), _isRegistered(other._isRegistered), _nickname(other._nickname), _username(other._username), _realname(other._realname), _channels(other._channels), _operatorChannels(other._operatorChannels)
{
}

Client &Client::operator=(const Client &other)
{
    if (this != &other)
    {
        _fd = other._fd;
        _recvBuffer = other._recvBuffer;
        _passAccepted = other._passAccepted;
        _isRegistered = other._isRegistered;
        _nickname = other._nickname;
        _username = other._username;
        _realname = other._realname;
        _channels = other._channels;
        _operatorChannels = other._operatorChannels;
    }
    return *this;
}

/* ==================== BUFFER ==================== */

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

/* ==================== PASSWORD ==================== */

bool Client::isPassAccepted() const
{
    return _passAccepted;
}

void Client::setPassAccepted(bool accepted)
{
    _passAccepted = accepted;
}

/* ==================== REGISTRATION ==================== */

bool Client::isRegistered() const
{
    return _isRegistered;
}

void Client::setRegistered(bool registered)
{
    _isRegistered = registered;
}

/* ==================== CLIENT INFO ==================== */

int Client::getFd() const
{
    return _fd;
}

const std::string &Client::getNickname() const
{
    return _nickname;
}

void Client::setNickname(const std::string &nickname)
{
    _nickname = nickname;
}

const std::string &Client::getUsername() const
{
    return _username;
}

void Client::setUsername(const std::string &username)
{
    _username = username;
}

const std::string &Client::getRealname() const
{
    return _realname;
}

void Client::setRealname(const std::string &realname)
{
    _realname = realname;
}

/* ==================== CHANNELS ==================== */

void Client::joinChannel(const std::string &channel)
{
    if (channel.empty())
        return;

    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (_channels[i] == channel)
            return;
    }

    _channels.push_back(channel);
}

void Client::partChannel(const std::string &channel)
{
    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (_channels[i] == channel)
        {
            _channels.erase(_channels.begin() + i);
            removeOperator(channel); // Remove operator status if the client leaves the channel
            return;
        }
    }
}

bool Client::isInChannel(const std::string &channel) const
{
    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (_channels[i] == channel)
            return true;
    }
    return false;
}

/* ==================== OPERATOR CHANNELS ==================== */

void Client::setOperator(const std::string &channel)
{
    for (size_t i = 0; i < _operatorChannels.size(); ++i)
    {
        if (_operatorChannels[i] == channel)
            return;
    }
    _operatorChannels.push_back(channel);
}

void Client::removeOperator(const std::string &channel)
{
    for (size_t i = 0; i < _operatorChannels.size(); ++i)
    {
        if (_operatorChannels[i] == channel)
        {
            _operatorChannels.erase(_operatorChannels.begin() + i);
            return;
        }
    }
}

bool Client::isOperator(const std::string &channel) const
{
    for (size_t i = 0; i < _operatorChannels.size(); ++i)
    {
        if (_operatorChannels[i] == channel)
            return true;
    }
    return false;
}