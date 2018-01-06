#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

pthread_mutex_t mtx;
pthread_spinlock_t spin;
pthread_rwlock_t rw;

int init(void);
void destroy(void);

void *some_ops_seq(void *arg);
void *reader_ops(void *arg);
void *writer_ops(void *arg);

void long_op(void);
void short_op(void);
void read_op(void);
void write_op(void);

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

    pthread_t thread1, thread2, reader, writer;

    pthread_create(&thread1, NULL, some_ops_seq, NULL);
    pthread_create(&thread2, NULL, some_ops_seq, NULL);
    pthread_create(&reader, NULL, reader_ops, NULL);
    pthread_create(&writer, NULL, writer_ops, NULL);
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(reader, NULL);
    pthread_join(writer, NULL);
    destroy();  
    return 0;
}

int init(void)
{
    return pthread_mutex_init(&mtx, NULL)
        || pthread_rwlock_init(&rw, NULL)
        || pthread_spin_init(&spin, PTHREAD_PROCESS_PRIVATE);
}

void destroy(void)
{
    pthread_mutex_destroy(&mtx);
    pthread_rwlock_destroy(&rw);
    pthread_spin_destroy(&spin);
}

void *some_ops_seq(void *arg)
{
    long_op();
    short_op();
    return NULL;
}

void *reader_ops(void *arg)
{
    read_op();
    return NULL;
}

void *writer_ops(void *arg)
{
    write_op();
    return NULL;
}

void long_op(void)
{
    if (pthread_mutex_lock(&mtx) == 0) {
        sleep(1);
        pthread_mutex_unlock(&mtx);
    }
}

void short_op(void)
{
    if (pthread_spin_lock(&spin) == 0) {
        usleep(1);
        pthread_spin_unlock(&spin);
    }
}

void read_op(void)
{
    if (pthread_rwlock_rdlock(&rw) == 0) {
        usleep(1);
        pthread_rwlock_unlock(&rw);
    }
}

void write_op(void)
{
    if (pthread_rwlock_wrlock(&rw) == 0) {
        usleep(1);
        pthread_rwlock_unlock(&rw);
    }
}

