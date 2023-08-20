#if !defined(log_h)
#define log_h

	#include <stdio.h>
	#include <stdlib.h>

	extern FILE *log_stream;

	#define LOG_STREAM (log_stream)

	void open_log_stream(const char *log_file_name);
	void vi_log(const char *log_format, ...);
	void close_log_stream();

#endif // log_h
