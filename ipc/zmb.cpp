#include <iostream>
#include <fstream>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void chl_hdlr(int signum);
void save_pid(const char *fname);

int main()
{
    signal(SIGCHLD, chl_hdlr);
    
    save_pid(fork() ? "pid_parent" : "pid_child");
    
    while (pause())
        ;
    return 0;
}

void save_pid(const char *fname)
{
    std::ofstream out(fname);
    out << getpid() << std::endl;
}

void chl_hdlr(int signum)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ; // only one instance of SIGCHLD can be queued
}

