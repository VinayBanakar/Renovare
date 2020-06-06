#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>

// https://blog.habets.se/2010/09/gettimeofday-should-never-be-used-to-measure-time.html

typedef long double u64;

/*
* Returns microsecond time difference in long double
*/
u64 getMicroDiff(struct timespec t2, struct timespec t1) {
	return ((t2.tv_nsec - t1.tv_nsec) * 0.001);
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
	struct timespec t1, t2;
	int fd;
	int iter = atoi(argv[1]);

	for( int i = 1; i <= iter; i++) {
	clock_gettime(CLOCK_MONOTONIC, &t1);
	fd = open("abc", O_RDONLY);
	clock_gettime(CLOCK_MONOTONIC, &t2);
	usec_open = movingAverage(usec_open, getMicroDiff(t2, t1), i);

	clock_gettime(CLOCK_MONOTONIC, &t1);
	read(fd,buf,0);
	clock_gettime(CLOCK_MONOTONIC, &t2);
	usec_read = movingAverage(usec_read, getMicroDiff(t2, t1), i);

	clock_gettime(CLOCK_MONOTONIC, &t1);
	close(fd);
	clock_gettime(CLOCK_MONOTONIC, &t2);
	usec_close = movingAverage(usec_close, getMicroDiff(t2, t1), i);
	}

	printf("======= clock_gettime =======\n");
	printf("Open: %Lf\n", usec_open);
	printf("Read: %Lf\n", usec_read);
	printf("Close: %Lf\n", usec_close);
}

