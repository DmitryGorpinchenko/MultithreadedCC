#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>

void chl_hdlr(int signum);

int main()
{
    int pair[2];
 
    signal(SIGCHLD, chl_hdlr);
    
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair)) {
        return -1;
    }
    
    int res = 0;
    
    if ((res = fork()) > 0) {
        close(pair[0]);
        while (pause())
            ;
        close(pair[1]);
    } else if (res == 0) {
        close(pair[1]);
        while (pause())
            ;
        close(pair[0]);
    } else {
        return -1;
    }

    return 0;
}

void chl_hdlr(int signum)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ; // only one instance of SIGCHLD can be queued
}
