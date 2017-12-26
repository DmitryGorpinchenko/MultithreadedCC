#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

int main(void)
{
    sem_t *sid = sem_open("/test.sem", O_CREAT, 0666, 66);
    
    if (sid == SEM_FAILED) {
        return 1;
    }

    sem_close(sid);
    return 0;
}

