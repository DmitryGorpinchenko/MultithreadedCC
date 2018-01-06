#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

#define BARRIER_COUNT 2

pthread_mutex_t mtx;
pthread_cond_t cond;
pthread_barrier_t bar;

int init(void);
void destroy(void);
void *worker1(void *arg);
void *worker2(void *arg);

int main(void)
{
    if (init() != 0) {
        return 1;
    }

    FILE *f = fopen("/home/box/main.pid", "w");
    
    if (f) {
        fprintf(f, "%d", getpid());
        fclose(f);
    }

    pthread_t thread1, thread2;

    pthread_create(&thread1, NULL, worker1, NULL);
    pthread_create(&thread2, NULL, worker2, NULL);
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    destroy();  
    return 0;
}

int init(void)
{
    return pthread_mutex_init(&mtx, NULL)
        || pthread_cond_init(&cond, NULL)
        || pthread_barrier_init(&bar, NULL, BARRIER_COUNT);
}

void destroy(void)
{
    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&cond);
    pthread_barrier_destroy(&bar);
}

void *worker1(void *arg)
{
    pthread_cond_wait(&cond, &mtx);
    usleep(1);
    pthread_barrier_wait(&bar);
    usleep(1);
    return NULL;
}

void *worker2(void *arg)
{
    usleep(1);
    pthread_cond_signal(&cond);
    usleep(1);
    pthread_barrier_wait(&bar);
    usleep(1);
    return NULL;
}
