#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

/*
* Three ways to solve problem 3:
* 1. sleep in rest of the parent.
* 2. sysctl kernel.sched_child_runs_first 
* This schedules child process first always.
* 3. use vfork()
* vfork() has always run the child first, since the parent won't even be runnable. 
* The parent wil get stuck in wait_for_completion(&vfork);
*/

int main() {
	static int sz;
	static char msg[10];
	int fd = open("tmp.txt", O_RDWR | O_CREAT, 0644);
	printf("Parent PID %d fd:%d\n", (int) getpid(), fd);

//	int rc = fork();
	int rc = vfork();
	if (rc < 0)
		fprintf(stderr, "fork failed\n");
	else if (rc ==0) {
		sprintf(msg, "%d", (int) getpid());
		msg[9] = '\n';
		sz = write(fd, msg ,10);	
		printf("Child PID %d fd:%d sz:%d\n", 
		(int) getpid(), fd, sz);
		printf("Hello\n");
		_exit(0);
	} else {
	// 	int rc_wait = wait(NULL);
		sprintf(msg, "%d", (int) getpid());
		msg[9] = '\n';
		sz = write(fd, msg ,10);	
		printf("Parent pid:%d fd:%d sz:%d\n",
		(int) getpid(), fd, sz);
		printf("Goodbye\n");
	}
	close(fd);
	return 0;
}
