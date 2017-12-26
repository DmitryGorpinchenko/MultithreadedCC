#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define IPC_PERMS 0666
#define MSG_SIZE 80

struct message {
    long mtype;
    char mtext[MSG_SIZE];
};

int main(void)
{
    key_t key = ftok("/tmp/msg.temp", 1);
    
    if (key < 0) {
        return 1;
    }
    
    int mq = msgget(key, IPC_CREAT | IPC_PERMS);
    
    if (mq < 0) {
        return 1;
    }
    
    struct message msg;
    int res = msgrcv(mq, &msg, MSG_SIZE, 0, 0);
    
    if (res < 0) { 
        return -1;
    }
    
    FILE *f = fopen("/home/box/message.txt", "w");
    fwrite(msg.mtext, sizeof(char), res, f);
    fclose(f);
    
    return 0;
}
