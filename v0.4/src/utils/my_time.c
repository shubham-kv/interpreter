#include "my_time.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

enum { NS_PER_SECOND = 1000000000 };

void calc_delta_time(const struct timespec *t1, const struct timespec *t2, struct timespec *td) {
	td->tv_nsec = t2->tv_nsec - t1->tv_nsec;
	td->tv_sec  = ((int64_t) t2->tv_sec) - ((int64_t) t1->tv_sec);

	if(td->tv_sec > 0 && td->tv_nsec < 0) {
		td->tv_nsec += NS_PER_SECOND;
		td->tv_sec--;
	}
	else if(td->tv_sec < 0 && td->tv_nsec > 0) {
		td->tv_nsec -= NS_PER_SECOND;
		td->tv_sec++;
	}
}


void put_formatted_time(char buffer[], const struct timespec *ts, size_t buffer_size) {
	uint32_t msec, musec, nsec;

	msec = ts->tv_nsec / 1000000l;
	nsec = ts->tv_nsec % 1000000l;
	musec = nsec / 1000;
	nsec = nsec % 1000;

	snprintf(buffer, buffer_size, "%lds %.3ldms %.3ld\xE6s %.3ldns", (int) ts->tv_sec, msec, musec, nsec);
}

