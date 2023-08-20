#include "parse_err_handler.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdint.h>

#include "main.h"


static const int MAX_ERROR_COUNT = 8;
static int cur_err_count = 0;

void flag_error(
	const int line_number,
	const char *err_line, const int err_pos, const char *err_msg, ...
) {
	va_list ap;
	va_start(ap, err_msg);

	fprintf(stderr, "\n");
	vfprintf(stderr, err_msg, ap);
	fprintf(stderr, "\n");
	va_end(ap);

	fprintf(stderr, "-%03d-| ", line_number);

	for(uint32_t i = 0; err_line[i] != '\0'; i++) {
		char c = err_line[i];
		
		if('\t' == c)
			c = ' ';

		fprintf(stderr, "%c", c);
	}

	// err_pos is assumed to be index
	for(int i=0; i < (err_pos + 7); i++) fprintf(stderr, " ");

	fprintf(stderr, "^\n\n");

	if(is_prompt_mode()) return;

	if(cur_err_count >= MAX_ERROR_COUNT) {
		fprintf(stderr, "\nToo many errors!\n");
		exit(EXIT_FAILURE);
	}

	cur_err_count++;
}


void flag_error_va(
	const int line_number,
	const char *err_line, const int err_pos, const char *err_msg,
	va_list *ap
) {
	fprintf(stderr, "\n");
	vfprintf(stderr, err_msg, *ap);
	fprintf(stderr, "\n");

	fprintf(stderr, "-%03d-| ", line_number);

	for(uint32_t i = 0; err_line[i] != '\0'; i++) {
		char c = err_line[i];
		
		if('\t' == c)
			c = ' ';

		fprintf(stderr, "%c", c);
	}

	// err_pos is assumed to be index
	for(uint32_t i = 0; i < (err_pos + 7); i++)
		fprintf(stderr, " ");

	fprintf(stderr, "^\n\n");

	if(is_prompt_mode()) return;

	if(cur_err_count >= MAX_ERROR_COUNT) {
		fprintf(stderr, "\nToo many errors!\n");
		exit(EXIT_FAILURE);
	}

	cur_err_count++;
}

/*
void report_error(
	const int line_number,
	const char *err_line, const int err_pos, const char *err_msg, ...
) {

	va_list ap;
	va_start(ap, err_msg);

	fprintf(stderr, "\n");
	vfprintf(stderr, err_msg, ap);
	fprintf(stderr, "\n");
	va_end(ap);


	fprintf(stderr, "-%03d-| ", line_number);

	for(uint32_t i = 0; err_line[i] != '\0'; i++) {
		char c = err_line[i];
		
		if('\t' == c)
			c = ' ';

		fprintf(stderr, "%c", c);
	}

	// err_pos is assumed to be index
	for(int i=0; i < (err_pos + 7); i++) fprintf(stderr, "_");

	fprintf(stderr, "^\n\n");

	if(cur_err_count >= MAX_ERROR_COUNT) {
		fprintf(stderr, "\nToo many errors!\n");
		exit(EXIT_FAILURE);
	}

	cur_err_count++;

	exit(EXIT_FAILURE);
}
*/
