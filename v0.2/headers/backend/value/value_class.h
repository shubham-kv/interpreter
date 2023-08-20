#if !defined(value_class_h)
#define value_class_h

	#include <stdint.h>

	// runtime representation of a class

	/*
	// v0.1
	struct ClassValue {
		const void *type;

		const void *symbol;
		const void *symbol_table;

		uint32_t property_count;
		void **properties;

		void *(*constructor)(const void *self, void **args);

		// hashtable of names to FunValues
		void *methods;

		uint32_t reference_count;
	};
	*/

	// v0.1
	struct ClassValue {
		const void *type;

		const void *symbol;
		const void *symbol_table;

		const void *super_class_symbol;

		uint32_t property_count;
		void **properties;

		void *(*constructor)(const void *self, void **args);

		// hashtable of names to FunValues
		void *methods;

		uint32_t reference_count;
	};

	extern const void *ClassValue;

	#define IS_CLASS_VALUE(ptr) ((ptr) && ((ClassValue) == ((struct ClassValue *) ptr)->type))

#endif // value_class_h
