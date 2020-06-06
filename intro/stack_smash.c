// echo python -c 'print("A"*60 + "\x00\x00\x00\x00")' | ./stack_smash badpass
// Overwriting result value by overflowing msg buffer
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
	int result = 1;

	char userPass[12];
	char* password = "password1";
	strcpy(userPass, argv[1]);
	
	result = strcmp(userPass, password);

	char msg[32];
	gets(msg);

	if(result == 0) {
		printf("Correct password!\n");
		printf("msg: %s\n",msg);
	} else {
		printf("incorrect!\n");
	}
	return 0;
}

/*
LLDB analysis:
gcc -g -o stack_smash stack_smash.c -Wall

// Start
lldb stack_smash
// Set param
settings set target.run-args badpass
b stack_smash.c:18
run
register read rax rsp rbp
register read
// to read buffer values
x/64x $rsp 
di -f
// Finish
continue

*/
