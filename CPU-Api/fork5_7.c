#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
	static int x = 100;
	printf("Parent PID %d x: %d\n", (int) getpid(), x);
	int rc = fork();
	if(rc <0) {
		fprintf(stderr, "fork failed\n");
		exit(1);
	} else if (rc ==0) {
		close(STDOUT_FILENO);
		//Child changes x
		x = 50;
		printf("Child PID %d x: %d\n", (int) getpid(), x);
		int ch_wait = wait(NULL);
		printf("ch_wait: %d\n", ch_wait);
		_exit(0);
	} else {
		int rc_wait = wait(NULL);
		printf("Parrent of %d [rc_wait:%d] [pid:%d] (x:%d)\n",
		rc, rc_wait, (int) getpid(), x);
	}
	return 0;
}
