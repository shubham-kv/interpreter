#if !defined(my_memory_h)
	#define my_memory_h

	#include <stddef.h>
	#include <stdint.h>

	void *allocate(size_t sizeof_each, uint64_t count);

	void *reallocate(void *array, size_t sizeof_each, uint64_t new_count);

#endif // my_memory_h
