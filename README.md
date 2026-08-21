*This project has been created as part of the 42 curriculum by kfuto, irene-diaz.*

# ft_irc - Internet Relay Chat Server

## Description

This project implements an **IRC (Internet Relay Chat) server** from scratch using **C++98**. The server handles multiple concurrent clients using non-blocking I/O and a single `poll()` for all operations, strictly following the IRC protocol specifications.

Key features:
- **Multi-client support**: Handle multiple clients simultaneously without forking
- **Non-blocking I/O**: All socket operations are non-blocking with proper polling
- **Channel management**: Users can create, join, and participate in channels
- **Channel operators**: Support for operator privileges with special commands
- **Channel modes**: Implement `+i` (invite-only), `+t` (topic restricted), `+k` (key/password), `+l` (user limit), `+o` (operator)
- **IRC commands**: PASS, NICK, USER, JOIN, PART, PRIVMSG, INVITE, TOPIC, MODE, KICK
- **Broadcasting**: Messages to channels are forwarded to all participants
- **C++ 98 compliant**: No external libraries or Boost

## Instructions

### Compilation
```bash
cd ft_irc
make
```

This generates an executable named `ircserv`.

### Execution
```bash
./ircserv <port> <password>
```

**Arguments:**
- `<port>`: The port number on which the server listens (e.g., 6667)
- `<password>`: The connection password required for all IRC clients

**Example:**
```bash
./ircserv 6667 secret
```

### Testing with an IRC Client

Connect using any IRC client (e.g., `nc`, `irssi`, or a graphical client):

```bash
nc 127.0.0.1 6667
```

Then send IRC commands:
```
PASS secret
NICK yourname
USER yourname 0 * :Your Real Name
JOIN #general
PRIVMSG #general :Hello everyone!
```

### Makefile Rules

- `make` or `make all`: Compile the project
- `make clean`: Remove object files
- `make fclean`: Remove object files and the executable
- `make re`: Clean and recompile

## Bonus: IRC Bot

A small server-side bot is included as a bonus feature. It responds to both direct messages and channel commands using a simple command set.

### Example commands

```text
PRIVMSG IRCBot :!help
PRIVMSG #general :!ping
PRIVMSG IRCBot :!time
```

The bot understands:
- `!help`: List the available commands
- `!time`: Show the server time
- `!ping`: Reply with a PONG
- `!whoami`: Tell the user which nickname they are using
- `!hello`: Greet the user

## Resources

### IRC Protocol Documentation
- **RFC 1459**: The original IRC protocol specification  
  https://tools.ietf.org/html/rfc1459
- **IRC Numerics Reference**: Error codes and replies  
  https://www.dal.net/docs/rfc1459.html
- **IRC Channel Modes**: Understanding channel flags  
  https://www.ircnow.org/index.php?title=Mode#Channel_Modes

### Socket Programming
- **man poll(2)**: Non-blocking I/O multiplexing
- **man socket(2)**: Socket creation and management
- **POSIX Socket Programming**: Standard networking APIs

### C++ 98 References
- **cppreference.com**: C++ standard library reference (filtered for C++98)
- **std::map, std::vector, std::string**: Containers used throughout the project

## AI Usage

**AI assistance was used for:**
1. **Implementation of core IRC commands** (INVITE, TOPIC, MODE) - Ensured correctness against RFC 1459 specifications
2. **Channel mode validation logic** - Proper enforcement of `+i`, `+t`, `+k`, `+l` restrictions
3. **Non-blocking I/O architecture** - Buffer management for send queues and POLLOUT handling
4. **Error handling and edge cases** - Graceful handling of client disconnections and partial message receiving
5. **Code review and optimization** - Ensuring C++ 98 compliance and clean architecture

## Project Structure

```
ft_irc/
├── Makefile
├── README.md
├── include/
│   ├── Server.hpp
│   ├── Client.hpp
│   ├── Channel.hpp
│   └── Parser.hpp
└── src/
    ├── main.cpp
    ├── Client.cpp
    ├── Channel.cpp
    ├── Parser.cpp
    └── Server/
        ├── Server.cpp
        ├── ServerCommands.cpp
        └── ServerNetwork.cpp
```

## Implementation Notes

- **Non-blocking sockets**: All client and server sockets are set to non-blocking mode using `fcntl()`
- **Single poll()**: All I/O multiplexing is done with one `poll()` call handling read, write, listen, and connection acceptance
- **Send buffer**: Client output is queued and flushed when POLLOUT is triggered
- **Channel cleanup**: Channels are automatically destroyed when no clients remain
- **Operator management**: Channel creators are automatically operators; operators can grant/revoke this privilege