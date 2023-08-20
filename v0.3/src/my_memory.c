#include "my_memory.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include "log.h"


void *allocate(size_t sizeof_each, uint64_t count) {
	#ifdef DEBUG
		vi_log("allocate(sizeof_each: %d, count: %d)\n", sizeof_each, count);
	#endif

	void *p = calloc(count, sizeof_each);

	if(NULL == p) {
		#ifdef DEBUG
			vi_log("MemoryAllocator: Failed to allocate memory!\n\n");
		#endif
		fprintf(stderr, "Not enough memory!\n");
		exit(EXIT_FAILURE);
	}

	return p;
}

void *reallocate(void *ptr, size_t sizeof_each, uint64_t new_count) {
	#ifdef DEBUG
		vi_log("reallocate(ptr: %x, sizeof_each: %d, new_count: %d)\n", ptr, sizeof_each, new_count);
	#endif

	if(new_count == 0) {
		free(ptr);
		return NULL;
	}

	void *new_ptr = realloc(ptr, new_count * sizeof_each);

	if(NULL == new_ptr) {
		#ifdef DEBUG
			vi_log("MemoryAllocator: Not enough memory!\n\n");
		#endif

		fprintf(stderr, "Not enough memory!\n");
		exit(EXIT_FAILURE);
	}

	return new_ptr;
}

