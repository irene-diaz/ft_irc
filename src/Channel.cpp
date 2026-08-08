#include "../include/Channel.hpp"

Channel::Channel()
    : _name(""), _inviteOnly(false), _topicRestricted(false), _topic(""), _password(""), _userLimit(0)
{
}

Channel::Channel(const std::string &name)
    : _name(name), _inviteOnly(false), _topicRestricted(false), _topic(""), _password(""), _userLimit(0)
{
}

Channel::Channel(const Channel &other)
    : _name(other._name), _clients(other._clients), _operators(other._operators),
      _invitedClients(other._invitedClients), _inviteOnly(other._inviteOnly), _topicRestricted(other._topicRestricted),
      _topic(other._topic), _password(other._password), _userLimit(other._userLimit)
{
}

Channel &Channel::operator=(const Channel &other)
{
    if (this != &other)
    {
        _name = other._name;
        _clients = other._clients;
        _operators = other._operators;
        _invitedClients = other._invitedClients;
        _inviteOnly = other._inviteOnly;
        _topicRestricted = other._topicRestricted;
        _topic = other._topic;
        _password = other._password;
        _userLimit = other._userLimit;
    }
    return *this;
}

const std::string &Channel::getName() const
{
    return _name;
}

Channel::~Channel()
{
}

void Channel::addClient(int fd)
{
    if (!hasClient(fd))
        _clients.push_back(fd);
}

void Channel::removeClient(int fd)
{
    for (size_t i = 0; i < _clients.size(); ++i)
    {
        if (_clients[i] == fd)
        {
            _clients.erase(_clients.begin() + i);
            removeOperator(fd); // Remove operator status if the client leaves the channel
            return;
        }
    }
}

bool Channel::hasClient(int fd) const
{
    for (size_t i = 0; i < _clients.size(); ++i)
    {
        if (_clients[i] == fd)
            return true;
    }
    return false;
}

void Channel::addOperator(int fd)
{
    if (!isOperator(fd))
        _operators.push_back(fd);
}

void Channel::removeOperator(int fd)
{
    for (size_t i = 0; i < _operators.size(); ++i)
    {
        if (_operators[i] == fd)
        {
            _operators.erase(_operators.begin() + i);
            return;
        }
    }
}

bool Channel::isOperator(int fd) const
{
    for (size_t i = 0; i < _operators.size(); ++i)
    {
        if (_operators[i] == fd)
            return true;
    }
    return false;
}

void Channel::addInvitedClient(int fd)
{
    for (size_t i = 0; i < _invitedClients.size(); ++i)
    {
        if (_invitedClients[i] == fd)
            return;
    }
    _invitedClients.push_back(fd);
}

void Channel::removeInvitedClient(int fd)
{
    for (size_t i = 0; i < _invitedClients.size(); ++i)
    {
        if (_invitedClients[i] == fd)
        {
            _invitedClients.erase(_invitedClients.begin() + i);
            return;
        }
    }
}

bool Channel::isInvitedClient(int fd) const
{
    for (size_t i = 0; i < _invitedClients.size(); ++i)
    {
        if (_invitedClients[i] == fd)
            return true;
    }
    return false;
}

bool Channel::isInviteOnly() const
{
    return _inviteOnly;
}

void Channel::setInviteOnly(bool value)
{
    _inviteOnly = value;
}

bool Channel::isTopicRestricted() const
{
    return _topicRestricted;
}

void Channel::setTopicRestricted(bool value)
{
    _topicRestricted = value;
}

const std::string &Channel::getTopic() const
{
    return _topic;
}

void Channel::setTopic(const std::string &topic)
{
    _topic = topic;
}

const std::string &Channel::getPassword() const
{
    return _password;
}

void Channel::setPassword(const std::string &password)
{
    _password = password;
}

int Channel::getUserLimit() const
{
    return _userLimit;
}

void Channel::setUserLimit(int limit)
{
    _userLimit = limit;
}

size_t Channel::getClientCount() const
{
    return _clients.size();
}