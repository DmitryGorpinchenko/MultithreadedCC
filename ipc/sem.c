#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define IPC_PERM 0666
#define SEMS_NUM 16

#if defined _SEM_SEMUN_UNDEFINED
union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
};
#endif

int main(void)
{
    key_t key = ftok("/tmp/sem.temp", 1);
    
    if (key < 0) {
        return 1;
    }

    int sid = semget(key, SEMS_NUM, IPC_CREAT | IPC_PERM);
    
    if (sid < 0) {
        return 1;
    }
    
    union semun arg;
    arg.array = malloc(sizeof(unsigned short) * SEMS_NUM);
    
    for (int i = 0; i < SEMS_NUM; i++) {
        arg.array[i] = i;
    }
    semctl(sid, 0, SETALL, arg);
    
    free(arg.array);
    return 0;
}
