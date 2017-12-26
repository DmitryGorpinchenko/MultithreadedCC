#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define IPC_PERM 0666
#define SHM_SIZE 1 << 20

int main(void)
{
    key_t key = ftok("/tmp/mem.temp", 1);
    
    if (key < 0) {
        return 1;
    }
    
    int shmid = shmget(key, SHM_SIZE, IPC_CREAT | IPC_PERM);

    if (shmid < 0) {
        return 1;
    }
    
    void *shm = shmat(shmid, NULL, 0);
    
    if (shm == (void *) (-1)) {
        return 1;
    }
    memset(shm, 42, SHM_SIZE);

    shmdt(shm);
    return 0;
}

