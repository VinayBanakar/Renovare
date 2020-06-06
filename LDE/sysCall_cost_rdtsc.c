// Average number of CPU cycles for system call
// Time taken can be CPU_cycles_taken/CPU_clock_speed [HZ]
// beware of turboboost
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

// http://developers.redhat.com/blog/2016/03/11/practical-micro-benchmarking-with-ltrace-and-sched/

/* One drawback of the RDTSC instruction is that the CPU is allowed to reorder 
   it relative to other instructions, which causes noise in our results. Fortunately, 
   Intel has provided an RDTSCP instruction that’s more deterministic. We’ll pair 
   that with a CPUID instruction which acts as a memory barrier, resulting in this: */

static __inline__ int64_t rdtsc_s(void)
{
  unsigned a, d; 
  asm volatile("cpuid" ::: "%rax", "%rbx", "%rcx", "%rdx");
  asm volatile("rdtsc" : "=a" (a), "=d" (d)); 
  return ((unsigned long)a) | (((unsigned long)d) << 32); 
}

static __inline__ int64_t rdtsc_e(void)
{
  unsigned a, d; 
  asm volatile("rdtscp" : "=a" (a), "=d" (d)); 
  asm volatile("cpuid" ::: "%rax", "%rbx", "%rcx", "%rdx");
  return ((unsigned long)a) | (((unsigned long)d) << 32); 
}

typedef long double ldb;

ldb movingAverage( ldb old_avg, ldb new_val, int count) {
	return (old_avg * (count - 1)/count + new_val/count);
}

int main(int argc, char* argv[]){
	int fd;
	char* buf[0];
	static ldb cyc_open =0, cyc_read = 0, cyc_close = 0;
	int iter = atoi(argv[1]);
	int64_t clocks_before, clocks_after;
	for( int i = 1; i <= iter; i++) {
	
	clocks_before = rdtsc_s();	
	fd = open("abc", O_RDONLY);
	clocks_after = rdtsc_e();
	cyc_open = movingAverage(cyc_open, (ldb) clocks_after - clocks_before, i);

	clocks_before = rdtsc_s();	
	read(fd,buf,0);
	clocks_after = rdtsc_e();
	cyc_read = movingAverage(cyc_read, (ldb) clocks_after - clocks_before, i);
	
	clocks_before = rdtsc_s();	
	close(fd);
	clocks_after = rdtsc_e();
	cyc_close = movingAverage(cyc_close, (ldb) clocks_after - clocks_before, i);
	}

	printf("======== CPU cycles ~ rdtsc ========\n");
	printf("Open: %Lf\n", cyc_open);
	printf("Read: %Lf\n", cyc_read);
	printf("close: %Lf\n", cyc_close);
}

