#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void delta_encode(unsigned char *buffer, int length)
{
    unsigned char last = 0;
    for (int i = 0; i < length; i++)
    {
        unsigned char current = buffer[i];
        buffer[i] = current - last;
        last = current;
    }
}

void delta_decode(unsigned char *buffer, int length)
{
    unsigned char last = 0;
    for (int i = 0; i < length; i++)
    {
        unsigned char delta = buffer[i];
        buffer[i] = delta + last;
        last = buffer[i];
    }
}

int main(){
	unsigned char buff[] = "Hello World\0";
	printf("Delta encoded: %s\n", buff);
	delta_encode(buff,12);
	for(int i = 0; i < 12; i++)
		printf("%d\n", buff[i]);
	printf("Decoding:\n");
	delta_decode(buff,12);
	for(int i=0; i<12; i++)
		printf("%d \t %c\n",buff[i], buff[i]);
	return 0;
}
