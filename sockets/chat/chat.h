#include <memory>

class ChatServer {
public:
    class Error {};

    ChatServer(short port);
    ~ChatServer();

    void run();
private:
    struct chat_impl;
    std::unique_ptr<chat_impl> chat;
};

