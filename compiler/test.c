#include <stdio.h>
#include <time.h>

extern int eval(int param);

#define NANO 1000000000
#define ITERATIONS 50000000

int main(int argc, char *argv) {
	/* Volatile to force evaluation each loop iteration */
	volatile int paramval = 0;
	volatile int result = 0;

	struct timespec ts1, ts2;
	while (scanf("%d", &paramval) == 1) {
		volatile int result = 0;
		clock_gettime(CLOCK_REALTIME, &ts1);
		for (int i = 0; i < ITERATIONS; i++) {
			result = eval(paramval);
		}
		clock_gettime(CLOCK_REALTIME, &ts2);
		long long nsec = ts2.tv_nsec;
		nsec -= ts1.tv_nsec;
		if (ts2.tv_sec > ts1.tv_sec)
			nsec += (ts2.tv_sec - ts1.tv_sec)*NANO;
		printf("Param=%d WCET=%d, time=%llu nsec for %u iters (%llu nsec/iter)\n", paramval, result, nsec, ITERATIONS, nsec/ITERATIONS);
	}
}
