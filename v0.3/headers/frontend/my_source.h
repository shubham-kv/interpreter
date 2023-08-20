#if !defined(source_h)
#define source_h

	#include <stdlib.h>
	#include <stdio.h>
	#include <stdint.h>
	#include <stdbool.h>

	struct Source {
		const void *type;

		// The name of the input stream [ file_name | stdin ]
		const char *instream;

		uint64_t lines_count;
		char **lines;
	};

	extern const void *Source;

	#define IS_SOURCE(ptr) ((ptr) && (((const struct Source *) ptr)->type == Source))

	#define SOURCE_INSTREAM(source)		(((const struct Source *) source)->instream)
	#define SOURCE_LINES_COUNT(source)	(((const struct Source *) source)->lines_count)
	#define SOURCE_LINES(source)		(((const struct Source *) source)->lines)
	#define SOURCE_GET_LINE(source, i)	(((const struct Source *) source)->lines[i])

	void source_read_file(void *source, const char *file_path);
	bool source_read_stream(void *source, FILE *stream);

#endif // source_h
