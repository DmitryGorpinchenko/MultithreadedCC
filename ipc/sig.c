#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    signal(SIGTERM, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    
    printf("%d\n", getpid());
    fclose(stdout);
    
    while (pause())
        ;
    return 0;
}
