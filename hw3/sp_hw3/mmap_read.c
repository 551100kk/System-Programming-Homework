/* mmap_write.c */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

typedef struct {
    char c_time_string[100];
} TimeInfo;

int main(int argc, char** argv) 
{
    int fd,i;
    char *p_map;
    
    const char  *file ="die_time";

    fd = open(file, O_RDWR);
    if(fd < 0) perror("open");
    p_map = (char*)mmap(0, 100,  PROT_READ,  MAP_SHARED, fd, 0);
    if (p_map == NULL) {
        puts("GG");
        return 0;
    }
    while(1) {
        sleep(2);
        printf("%s\n", p_map);
    }

    return 0;
}