#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>

int main(void)
{
    int rfd, wfd, res;
    char buf[PIPE_BUF];

    if (mkfifo("/home/box/in.fifo", 0666) < 0 || mkfifo("/home/box/out.fifo", 0666) < 0) {
        return -1;
    }

    rfd = open("/home/box/in.fifo", O_RDONLY);
    wfd = open("/home/box/out.fifo", O_WRONLY);

    if (rfd < 0 || wfd < 0) {
        return -1;
    }

    while ((res = read(rfd, buf, PIPE_BUF)) > 0) {
        write(wfd, buf, res);
    }
    return 0;
}

