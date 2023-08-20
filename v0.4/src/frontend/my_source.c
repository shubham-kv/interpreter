#include "my_source.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my_memory.h"
#include "type.h"

#define LINE_BUFFER_SIZE 1024
#define LINE_STEP_SIZE 64



static void *source_constructor(void *self, va_list *ap);
static void *source_destructor(void *self);

const struct Type _type_source = {
	sizeof(struct Source),
	source_constructor,
	source_destructor
};

const void *Source = &_type_source;


static void *source_constructor(void *_self, va_list *ap) {
	struct Source *self = _self;
	assert(IS_SOURCE(self));

	self->instream = va_arg(*ap, const char *);

	self->lines_count = 0;
	self->lines = NULL;

	return self;
}

static void *source_destructor(void *_self) {
	struct Source *self = _self;
	assert(IS_SOURCE(self));

	if(self->lines) {
		for(int i = 0; i < self->lines_count; i++) {
			free(self->lines[i]);
		}

		free(self->lines);
	}

	return self;
}








void source_read_file(void *source, const char *file_path) {
	assert(IS_SOURCE(source));
	assert(file_path);

	struct Source *self = source;
	FILE *stream = fopen(file_path, "rb");

	if(!stream) {
		fprintf(stderr, "Cannot read file '%s'.\n", file_path);
		exit(EXIT_FAILURE);
	}

	char **lines = NULL;
	uint64_t lines_size = 0;
	uint64_t lines_count = 0;

	char line_buffer[LINE_BUFFER_SIZE];

	while(fgets(line_buffer, LINE_BUFFER_SIZE, stream)) {
		if(lines_count == lines_size) {
			lines_size += LINE_STEP_SIZE;
			lines = reallocate(lines, sizeof(char *), lines_size);
		}

		size_t line_len = strlen(line_buffer);
		char *line = allocate(sizeof(char), line_len + 1);

		strncpy(line, line_buffer, line_len);
		lines[lines_count++] = line;
	}

	if(lines_count < lines_size) {
		// free the unused memory
		lines = reallocate(lines, sizeof(char *), lines_count);
	}

	self->lines = lines;
	self->lines_count = lines_count;

	fclose(stream);
}

bool source_read_stream(void *source, FILE *stream) {
	assert(IS_SOURCE(source));
	assert(stream);

	struct Source *self = source;
	char line_buffer[LINE_BUFFER_SIZE];

	fprintf(stdout, ">> ");
	fflush(stdout);

	if(fgets(line_buffer, LINE_BUFFER_SIZE, stream)) {
		if(strlen(line_buffer) > 1) {
			self->lines = reallocate(self->lines, sizeof(char *), ++self->lines_count);
			size_t line_len = strlen(line_buffer);

			char *line = allocate(sizeof(char), line_len + 1);
			strncpy(line, line_buffer, line_len);

			self->lines[self->lines_count - 1] = line;

			return true;
		}
		else {
			return false;
		}
	}
	else {
		exit(EXIT_FAILURE);
	}
}

