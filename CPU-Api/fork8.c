#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
	printf("Parent: %d\n", (int) getpid());
	pid_t pid1, pid2;
	// two ends of the pipe
	int fd[2];
	char buf[20];

	if(pipe(fd) < 0) {
		perror("pipe");
		exit(1);
	}
	
	pid1 = fork();
	if(pid1 ==0) {
		close(fd[0]);
		char *message = "Hello from child";
		size_t len = strlen(message);
		write(fd[1], message, len);
		printf("CH: %d sent msg: %s\n", (int) getpid(), message);
		_exit(0);
	}

	pid2 = fork();
	if(pid2 ==0) {
		close(fd[1]);
		size_t cnt = read(fd[0], buf, sizeof(buf)-1);
		if (cnt <= 0) {
			perror("read");
			exit(1);
		}
		buf[cnt] = '\0';
		printf("CH: %d recieved msg: %s\n", (int) getpid(), buf);
		_exit(0);
	}
	return 0;
}
