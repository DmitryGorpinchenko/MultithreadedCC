#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <iostream>
#include <set>
#include <algorithm>
#include <memory>

#include "chat.h"

static const int MAX_MSG_SIZE    = 1024;
static const int MAX_EVENTS      = 32;
static const int MAX_HEADER_SIZE = 10;

template <typename sp> struct shared_cmp { bool operator()(sp l, sp r) const { return (l && r) ? (*l < *r) : (l < r); } };

struct endpoint {
    endpoint(int socket) : socket_(socket), valid_(socket >= 0) {}
    ~endpoint() { if (valid_) { close(socket_); } }

    endpoint(const endpoint &rhs) = delete;
    endpoint(endpoint &&rhs) = delete;
    endpoint &operator=(const endpoint &rhs) = delete;
    endpoint &operator=(endpoint &&rhs) = delete;

    std::string recieve(int &err_code) const {
        const auto res = recv(socket_, buf_, MAX_MSG_SIZE, 0);
        err_code = res >= 0 ? 0 : errno;
        return std::string(buf_, res > 0 ? res : 0);
    }
    void send(const std::string &hdr, const std::string &msg) const {
        snprintf(buf_, MAX_MSG_SIZE, "%s: %s\n", hdr.c_str(), msg.c_str());
        ::send(socket_, buf_, strlen(buf_), MSG_NOSIGNAL);
    }
    operator bool() const { return valid_; }
    operator int() const { return socket_; }
    bool operator<(const endpoint &other) const { return socket_ < other.socket_; }
private:
    int socket_;
    bool valid_;
    mutable char buf_[MAX_MSG_SIZE];
};
using shared_end = std::shared_ptr<endpoint>;
using shared_end_cmp = shared_cmp<shared_end>;

struct acceptor {
    int epoll;

    acceptor(short port)
        : epoll(epoll_create1(0))
        , master_(new endpoint(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)))
    {
        sockaddr_in addr;
        addr.sin_family      = AF_INET;
        addr.sin_port        = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        bzero(&ev_, sizeof(epoll_event));
        ev_.events           = EPOLLIN;
        ev_.data.fd          = *master_;
        if ((epoll < 0)
        || !(*master_)
        ||  (bind(*master_, (sockaddr *) (&addr), sizeof(addr)) < 0)
        ||  (listen(*master_, SOMAXCONN) < 0)
        ||  (epoll_ctl(epoll, EPOLL_CTL_ADD, *master_, &ev_) < 0)) {

            throw ChatServer::Error();
        }
    }
    ~acceptor() { close(epoll); }

    operator int() const { return *master_; }
    operator shared_end() const { return master_; }

    shared_end accept(int &err_code) const {
        shared_end end(new endpoint(::accept4(*master_, nullptr, nullptr, SOCK_NONBLOCK)));

        ev_.events  = EPOLLIN;
        ev_.data.fd = *end;

        if (*end) {
            if (!epoll_ctl(epoll, EPOLL_CTL_ADD, *end, &ev_)) {
                err_code = 0;
                return end;
            }
        }
        
        err_code = errno;
        return {};
    }
private:
    shared_end master_;
    mutable epoll_event ev_;
};

struct ChatServer::chat_impl {
    chat_impl(short port);
    void run();
private:
    void add_client(shared_end client);
    void remove_client(shared_end client);
    void broadcast_msg(shared_end from, const std::string &msg) const;
    
    acceptor master_;
    std::set<shared_end, shared_end_cmp> slaves_;
};

ChatServer::ChatServer(short port) : chat(new chat_impl(port)) {}
ChatServer::~ChatServer() = default;

void ChatServer::run() { chat->run(); }

void ChatServer::chat_impl::run()
{
    int ret_events = 0, err_code = 0;
    epoll_event events[MAX_EVENTS];

    while ((ret_events = epoll_wait(master_.epoll, events, MAX_EVENTS, -1)) >= 0) {
        for (int i = 0; i < ret_events; ++i) {
            const auto &ev = events[i];
            if (ev.data.fd == master_) {
                do {
                    add_client(master_.accept(err_code));
                    if (err_code == EAGAIN || err_code == EWOULDBLOCK) {
                        break; // tried to accept all pending connections
                    }
                } while (true);
            } else {
                const auto begin = slaves_.begin();
                const auto end = slaves_.end();
                const auto it = std::find_if(begin, end, [&ev](auto slave) { return *slave == ev.data.fd; });

                if (it != end) {
                    const auto msg = (*it)->recieve(err_code);

                    if (!err_code) {
                        if (!msg.empty()) {
                            broadcast_msg(*it, msg);
                        } else {
                            remove_client(*it);
                        }
                    }
                }
            }
        }
    }
}

ChatServer::chat_impl::chat_impl(short port) : master_(port) {}

void ChatServer::chat_impl::broadcast_msg(shared_end from, const std::string &msg) const
{
    for (const auto &slave : slaves_) { if (slave != from) { slave->send(std::to_string(int(*from)), msg); } }
}
void ChatServer::chat_impl::add_client(shared_end client)
{
    if (client && *client) {
        if (slaves_.insert(client).second) {
            broadcast_msg(client, "joined conversation");
        }
    }
}
void ChatServer::chat_impl::remove_client(shared_end client)
{
    if (slaves_.erase(client)) {
        broadcast_msg(client, "leaved conversation");
    }
}

