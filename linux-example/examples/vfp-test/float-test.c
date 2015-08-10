#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#ifndef timersub
#define timersub(a, b, result) \
	do { \
		(result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
		(result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
		if ((result)->tv_usec < 0) { \
			--(result)->tv_sec; \
			(result)->tv_usec += 1000000; \
		} \
	} while (0)
#endif


int main(void) {
	float a[10];
	float b[10];
	float r[10];
	struct timeval tv1, tv2, result;
	int i, j;

	printf("A: ");
	for (i = 0; i < 10; i++) {
		a[i] = (100.0 * (rand() / (RAND_MAX + 1.0)));
		printf(" %12.3f", a[i]);
	}
	printf("\n");

	printf("B: ");
	for (i = 0; i < 10; i++) {
		b[i] = (20.0 * (rand() / (RAND_MAX + 1.0)));
		printf(" %12.3f", b[i]);
	}
	printf("\n");

	gettimeofday(&tv1, NULL);

	for (j = 0; j < 100000; j++) {
		for (i = 0; i < 10; i++) {
			r[i] = a[i] * b[i];
			a[i] += 0.1;
			b[i] += 0.2;
		}
	}
	for (i = 0; i < 10; i++) {
		r[i] /= 119.61089623f;
	}

	gettimeofday(&tv2, NULL);

	printf("R: ");
	for (i = 0; i < 10; i++) {
		printf(" %12.3f", r[i]);
	}
	printf("\n");

	timersub(&tv2, &tv1, &result);
	printf("S: %12d.%06d\n", tv1.tv_sec, tv1.tv_usec);
	printf("E: %12d.%06d\n", tv2.tv_sec, tv2.tv_usec);
	printf("E: %12d.%06d\n", result.tv_sec, result.tv_usec);

	return 0;
}

