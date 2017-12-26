#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define SHM_SIZE 1 << 20

int main(void)
{
    int shmfd = shm_open("/test.shm", O_RDWR | O_CREAT, 0666);
    
    if (shmfd < 0 || ftruncate(shmfd, SHM_SIZE) < 0) {
        return 1;
    }
    
    void *shm = mmap(NULL,
                     SHM_SIZE,
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED,
                     shmfd,
                     0);
                     
    if (shm == MAP_FAILED) {
        return 1;
    }
    memset(shm, 13, SHM_SIZE);
    
    munmap(shm, SHM_SIZE);
    return 0;
}
