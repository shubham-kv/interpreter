#include "memory.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>


void *allocate(size_t sizeof_each, uint32_t count) {
	void *p = calloc(count, sizeof_each);

	if(NULL == p) {
		fprintf(stderr, "Failed to allocate memory!\n");
		exit(EXIT_FAILURE);
	}

	return p;
}


void *reallocate(void *ptr, size_t sizeof_each, uint32_t new_count) {
	if(new_count == 0) {
		free(ptr);
		return NULL;
	}

	void *new_array = realloc(ptr, new_count * sizeof_each);

	if(NULL == new_array) {
		fprintf(stderr, "Not enough memory!\n");
		exit(EXIT_FAILURE);
	}

	return new_array;
}


