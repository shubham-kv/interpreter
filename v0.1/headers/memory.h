#if !defined(memory_h)
	#define memory_h

	#include <stddef.h>
	#include <stdint.h>

	void *allocate(size_t sizeof_each, uint32_t count);

	void *reallocate(void *array, size_t sizeof_each, uint32_t new_count);

#endif // memory_h
