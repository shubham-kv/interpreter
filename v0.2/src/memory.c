#include "memory.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include "log.h"


void *allocate(size_t sizeof_each, uint32_t count) {
	#ifdef DEBUG
		vi_log("allocate(sizeof_each: %d, count: %d)\n", sizeof_each, count);
	#endif

	void *p = calloc(count, sizeof_each);

	if(NULL == p) {
		#ifdef DEBUG
			fprintf(stderr, "MemoryAllocator: Failed to allocate memory!\n");
		#endif

		exit(EXIT_FAILURE);
	}

	#ifdef DEBUG
		vi_log("ret allocate() memory allocated from %x\n", p);
	#endif

	return p;
}


void *reallocate(void *ptr, size_t sizeof_each, uint32_t new_count) {
	#ifdef DEBUG
		vi_log("reallocate(ptr: %x, sizeof_each: %d, new_count: %d)\n", ptr, sizeof_each, new_count);
	#endif

	if(new_count == 0) {
		#ifdef DEBUG
			vi_log("ret reallocate(): memory deallocated at %x\n", ptr);
		#endif

		free(ptr);
		return NULL;
	}

	void *new_array = realloc(ptr, new_count * sizeof_each);

	if(NULL == new_array) {
		#ifdef DEBUG
			vi_log("MemoryAllocator: Not enough memory!\n");
		#endif
		
		exit(EXIT_FAILURE);
	}

	#ifdef DEBUG
		vi_log("ret reallocate(): memory reallocated at %x\n", new_array);
	#endif

	return new_array;
}


