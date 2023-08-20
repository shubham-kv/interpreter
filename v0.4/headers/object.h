#if !defined(object_h)
	#define object_h

	#include <stdbool.h>

	// generic method to create a new object of 'struct Type'
	void *new(const void *class, ...);

	// generic method that deletes an object
	void delete(void *self);

	// int equals(const void *_self, const void *_other);

#endif // object_h
