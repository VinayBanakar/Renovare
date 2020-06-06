/*
* vsyscall and VSDO serve the same functionality, however vsyscall is limited to 4 entries (gettimeofday, time, getcpu) and is static in memory where as vdso is dynamically loaded hence different memory range for each process because of ASLR.
*/

// do get time of date check asm

#include <time.h>
#include <sys/time.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    char buffer[40];
    struct timeval time;

    gettimeofday(&time, NULL);

    strftime(buffer, 40, "Current date/time: %m-%d-%Y/%T", localtime(&time.tv_sec));
    printf("%s\n",buffer);

    return 0;
}

/*
Perhaps a wrong question, but why aren't there more virtual system calls supported today when we know context switching performance overhead is why people are moving to kernel bypass.
It's not really worth it unless the system call gets called often enough to make a difference, is fast enough for the saving to be worthwhile and is suitable for a VDSO implementation.
suitable is things like needing to be read only and being able to put the data to be read in a vdso page in a sensible fashion without overheads.
*/
