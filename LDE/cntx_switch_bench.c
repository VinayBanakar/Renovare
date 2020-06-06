// A way to measure context switch cost in time between two process
// taskset -c 0 ./cntx_sw_bench 100000

/*
* Syscall noise is still not accounted for.
* Alternate: perf record -e context-switches -a
* futex (stay in uspace as much as possible by calling fewer system calls) 
* over mutex for reduced U-K transition overhead.
*/
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <math.h>

#define RE 0 // read end
#define WE 1 // write end
typedef long double ldb;
static ldb* metrics;

/*
* Returns new moving average - algo expects count change by one value
*/
ldb movingAverage( ldb old_avg, ldb new_val, int count) {
        return (old_avg * (count - 1)/count + new_val/count);
}

int main(int argc, char* argv[]){
	pid_t rc1, rc2, wpid;
	int status =0;
	int fd1[2];
	int fd2[2];
	char buf1[12], buf2[12];
	char* msg = "Hello world";
	
	// time related
	struct timespec t1, t2;
	metrics = mmap(NULL, 4*sizeof(*metrics), 
		PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
	ldb ctx_sw_cost = 0;
	


	if( pipe(fd1) || pipe(fd2) < 0) {
		perror("pipe");
		exit(1);
	}
	int iter = atoi(argv[1]);
	
	for(int i = 1; i <= iter; ++i) {

	rc1 = fork();
	if(rc1 == 0) {
		// Context switched to write.
		clock_gettime(CLOCK_MONOTONIC, &t1);
		metrics[0] =  t1.tv_nsec;
		//p1 writes to pipe 1
		close(fd1[RE]);
		write(fd1[WE], msg, strlen(msg));
		// and waits for a read on the 2nd pipe
		close(fd2[WE]);
		clock_gettime(CLOCK_MONOTONIC, &t2);
		metrics[3] = t2.tv_nsec;
		size_t cnt = read(fd2[RE],buf1, sizeof(buf1)-1);
		if (cnt <= 0) {
			perror("read");
			exit(1);
		}
		buf1[cnt] = '\0';
		_exit(0);
	}

	rc2 = fork();
	if(rc2 == 0) {
		clock_gettime(CLOCK_MONOTONIC, &t2);
		metrics[2] =  t2.tv_nsec;
                //p2 writes to pipe 2
                close(fd2[RE]);
                write(fd2[WE], msg, strlen(msg));
                
		// and waits for a read on the 1st pipe
                close(fd1[WE]);
		clock_gettime(CLOCK_MONOTONIC, &t1);
		metrics[1] = t1.tv_nsec;
                size_t cnt = read(fd1[RE],buf2, sizeof(buf2)-1);
                if (cnt <= 0) {
                        perror("read");
                        exit(1);
                }
		buf2[cnt] = '\0';
		_exit(0);
	}
	
	while ((wpid = wait(&status)) > 0);
	ctx_sw_cost =  movingAverage(ctx_sw_cost, fabsl(metrics[0] - metrics[1]), i);
	ctx_sw_cost =  movingAverage(ctx_sw_cost, fabsl(metrics[2] - metrics[3]), i);
	
	}
	printf("Average context switch cost: %Lf us - %d iterations\n", ctx_sw_cost * 0.001, iter);
}
