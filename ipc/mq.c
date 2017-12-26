#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

void exit_mq(const char *name, mqd_t mqd, int code)
{
    mq_close(mqd);
    mq_unlink(name);
    exit(code);
}

const char *name = "/test.mq";

int main(void)
{
    mqd_t mqd = mq_open(name, O_RDWR | O_CREAT, 0666, NULL);
    
    if (mqd == ((mqd_t) -1)) {
        return 1;
    }
    
    struct mq_attr attr;
    if (mq_getattr(mqd, &attr) < 0) {
        exit_mq(name, mqd, 1);
    }
    
    char buf[attr.mq_msgsize];
    ssize_t res = mq_receive(mqd, buf, attr.mq_msgsize, NULL);
    
    if (res < 0) {
        exit_mq(name, mqd, 1);
    }
    
    FILE *f = fopen("/home/box/message.txt", "w");
    fwrite(buf, sizeof(char), res, f);
    fclose(f);
    
    mq_close(mqd);
    mq_unlink(name);
    return 0;
}

