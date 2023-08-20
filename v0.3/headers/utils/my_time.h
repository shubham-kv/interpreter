#if !defined(my_time_h)
#define my_time_h

	#include <unistd.h>
	#include <time.h>
	#include <stddef.h>

	void calc_delta_time(const struct timespec *t1, const struct timespec *t2, struct timespec *td);

	void put_formatted_time(char buffer[], const struct timespec *ts, size_t buffer_size);

#endif // my_time_h
