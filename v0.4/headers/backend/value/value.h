#if !defined(value_h)
	#define value_h

	#include <stdbool.h>
	#include <stdint.h>

	#include "value_type.h"

	struct Value {
		const void *type;

		enum ValueType value_type;

		union {
			long long long_val;
			int integer_val;
			double double_val;
			void *pointer;
		} value;

		// the total count of references to this value
		uint64_t reference_count;
	};

	// a circular list of all distinct literal strings
	// struct Value *string_ring;

	extern const void *Value;

	#define IS_VALUE(ptr) ((ptr) && (((const struct Value *) ptr)->type == Value))

	#define GET_VALUE_TYPE(value_ptr) (((struct Value *) value_ptr)->value_type)

	#define VALUE_AS_INT(value_ptr) 	(((struct Value *) value_ptr)->value.integer_val)
	#define VALUE_AS_LONG(value_ptr) 	(((struct Value *) value_ptr)->value.long_val)
	#define VALUE_AS_DOUBLE(value_ptr) 	(((struct Value *) value_ptr)->value.double_val)
	#define VALUE_AS_STRING(value_ptr) 	(((struct Value *) value_ptr)->value.pointer)
	#define VALUE_AS_PTR(value_ptr) 	(((struct Value *) value_ptr)->value.pointer)

#endif // value_h
