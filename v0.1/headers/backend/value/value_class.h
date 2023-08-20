#if !defined(value_class_h)
#define value_class_h

	#include <stdint.h>

	// runtime representation of a class
	struct ClassValue {
		const void *type;

		const void *symbol;
		const void *symbol_table;

		uint32_t property_count;
		void **properties;

		void *methods;

		void *(*constructor)(const void *self, void **args);

		uint32_t reference_count;
	};

	extern const void *ClassValue;

	#define IS_CLASS_VALUE(ptr) ((ptr) && ((ClassValue) == ((struct ClassValue *) ptr)->type))

#endif // value_class_h
