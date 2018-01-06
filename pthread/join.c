#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

void *start_routine(void *arg)
{
    sleep(10);
    return NULL;
}

int main(void)
{
    FILE *f = fopen("/home/box/main.pid", "w");
    fprintf(f, "%d", getpid());
    fclose(f);

    pthread_t thread;

    if (pthread_create(&thread, NULL, start_routine, NULL) != 0) {
        return 1;
    }
    
    pthread_join(thread, NULL);    
    return 0;
}

