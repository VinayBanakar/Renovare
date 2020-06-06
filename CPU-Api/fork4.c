//#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

// exec*p uses $PATH
// https://linuxhint.com/exec_linux_system_call_c/

int main() {
	printf("Parent: %d\n", (int) getpid());
	int status =0;

	pid_t rc1 = fork(); 
	if (rc1 ==0) {
		char *args[] = {"/bin/ls", "-ll", NULL};
		execvp(args[0], args);
		printf("failed to invoke execvp\n");
		_exit(0);
	}
	pid_t rc2 = fork();
	if (rc2 ==0) {
		char *args[] = {"/bin/sh", "-c", (char *) "whoami", NULL};
		execv(args[0], args);
		printf("failed to invoke execv\n");
		_exit(0);
	}
	pid_t rc3 = fork();
	if(rc3 ==0) {
		execl("fork1", "fork1", NULL, NULL, NULL);
		printf("failed to invoke execl\n");
	}
	waitpid(rc2, &status, 0);
	printf("status: %d\n", status);
	return 0;
}
