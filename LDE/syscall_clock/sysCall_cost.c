#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>


typedef long double u64;

/*
* Returns microsecond time difference in long double
*/
u64 getMicroDiff(struct timeval t2, struct timeval t1) {
	return ((1000000*t2.tv_sec) + t2.tv_usec) - ((1000000*t1.tv_sec) + t1.tv_usec);
}

/*
* Returns new moving average - algo expects count change by one value
*/
u64 movingAverage( u64 old_avg, u64 new_val, int count) {
	return (old_avg * (count - 1)/count + new_val/count);
}

int main(int argc, char* argv[]) {
	static u64 usec_open =0, usec_read = 0, usec_close = 0;
	char* buf[0];
	struct timeval t1, t2;
	int fd;
	int iter = atoi(argv[1]);

	for( int i = 1; i <= iter; i++) {
	gettimeofday(&t1, NULL);
	fd = open("abc", O_RDONLY);
	gettimeofday(&t2, NULL);
	usec_open = movingAverage(usec_open, getMicroDiff(t2, t1), i);

	gettimeofday(&t1, NULL);
	read(fd,buf,0);
	gettimeofday(&t2, NULL);
	usec_read = movingAverage(usec_read, getMicroDiff(t2, t1), i);

	gettimeofday(&t1, NULL);
	close(fd);
	gettimeofday(&t2, NULL);
	usec_close = movingAverage(usec_close, getMicroDiff(t2, t1), i);
	}


	printf("Open: %Lf\n", usec_open);
	printf("Read: %Lf\n", usec_read);
	printf("Close: %Lf\n", usec_close);
}
