#define _GNU_SOURCE
#include<stdio.h>
#include <unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<string.h>
#include<stdint.h>
extern int errno;
int main()
{
    int errnum;
    char bigbuffer[1536];
    // On O_DIRECT writes must be 512 algined
    char *buf1 = (char *)(((uintptr_t)bigbuffer + 511) & ~511);
    strcpy(buf1, "hello, world");
    int fd = open("foo.txt", O_CREAT | O_WRONLY |O_DIRECT | O_SYNC, 00666);
    int ret = write(fd, buf1, 512);
    if(ret<0){
    	errnum = errno;
	fprintf(stderr, "Error opening file: %s\n", strerror( errnum ));
    }
    close(fd);
    return 0;
}
