#include "../../include/Server.hpp"

// default value for _serverFd is -1, which indicates that the server is not yet initialized
Server::Server() : _serverFd(-1), _isRunning(false)
{
}

Server::~Server()
{
    for (std::map<int, Client>::iterator it = _clients.begin();
         it != _clients.end();
         ++it)
    {
        close(it->first);
    }

    if (_serverFd != -1)
        close(_serverFd);
}

// ANTES DE ENTREGAR REVISAR SI REALMENTE SON NECESARIOS LOS CONSTRUCTORES DE COPIA Y ASIGNACION, YA QUE NO SE USAN EN NINGUN LADO Y NO TIENEN SENTIDO PARA UN SERVIDOR, YA QUE NO SE DEBERIA COPIAR UN SERVIDOR, SINO QUE SE DEBERIA CREAR UNO NUEVO CON SUS PROPIOS PARAMETROS. SI NO SE USAN, PONERLOS PRIVADOS SIN IMPLEMENTACION.
Server::Server(const Server &other)
    : _serverFd(other._serverFd),
      _serverAddr(other._serverAddr),
      _isRunning(other._isRunning),
      _parser(other._parser),
      _password(other._password),
      _pollfds(other._pollfds),
      _clients(other._clients)
{
}

Server &Server::operator=(const Server &other)
{
    if (this != &other)
    {
        _serverFd = other._serverFd;
        _serverAddr = other._serverAddr;
        _isRunning = other._isRunning;
        _parser = other._parser;
        _password = other._password;
        _pollfds = other._pollfds;
        _clients = other._clients;
    }
    return *this;
}
