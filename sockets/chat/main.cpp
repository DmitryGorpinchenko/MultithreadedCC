#include <sstream>

#include "chat.h"

int main(int argc, char **argv)
{
    short port;
    if (argc == 2 && (std::istringstream(argv[1]) >> port)) {
        try {
            ChatServer(port).run();
        } catch(...) {
            return -1;
        }
    }
    return 0;
}

