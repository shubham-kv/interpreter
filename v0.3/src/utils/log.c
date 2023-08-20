#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>


FILE *log_stream = NULL;

void open_log_stream(const char *log_file_name) {
	log_stream = fopen(log_file_name, "wb");
}

void close_log_stream() {
	if(NULL != log_stream) {
		fclose(log_stream);
	}
}

void vi_log(const char *log_format, ...) {
	// time_t start_time;
	// time(&start_time);
	// struct tm *start_tm = localtime(&start_time);


	va_list ap;
	va_start(ap, log_format);

	if(NULL != log_stream) {

		// time_t timer = time(NULL);

		// if(timer != ((time_t) -1)) {
		// 	char buffer[128];
		// 	if(strftime(buffer, sizeof buffer, "[%d/%m/%y %H:%M:%S]", localtime(&timer))) {
		// 		fprintf(log_stream, "%s ", buffer);
		// 	} else {
		// 		fprintf(log_stream, "[ NULL ] ");
		// 	}
		// }

		vfprintf(log_stream, log_format, ap);

		fflush(log_stream);
	}

	va_end(ap);

	// time_t end_time;
	// time(&end_time);
	// printf("Total Scanning time: %fs\n", difftime(end_time, start_time));	
}

