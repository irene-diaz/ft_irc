#include <string>
#include <vector>
class Channel
{
private:
    std::string _name;

    std::vector<int> _clients;
    std::vector<int> _operators;
    std::vector<int> _invitedClients;

    bool _inviteOnly;      //+i(invite-only)
    bool _topicRestricted; //+t(topic restricted)

    std::string _topic;
    std::string _password; //+k(password)
    int _userLimit;        //+l(user limit)

public:
    Channel();
    Channel(const std::string &name);
    Channel(const Channel &other);
    Channel &operator=(const Channel &other);
    ~Channel();

    const std::string &getName() const;

    void addClient(int fd);
    void removeClient(int fd);

    bool hasClient(int fd) const;

    void addOperator(int fd);
    void removeOperator(int fd);
    bool isOperator(int fd) const;

    bool isInviteOnly() const;
    void setInviteOnly(bool value);

    bool isTopicRestricted() const;
    void setTopicRestricted(bool value);

    const std::string &getTopic() const;
    void setTopic(const std::string &topic);

    const std::string &getPassword() const;
    void setPassword(const std::string &password);

    int getUserLimit() const;
    void setUserLimit(int limit);

    void addInvitedClient(int fd);
    void removeInvitedClient(int fd);
    bool isInvitedClient(int fd) const;

    size_t getClientCount() const;
};