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

	#define IS_VALUE(obj) ((obj) && (((struct Value *) obj)->type == Value))

	#define GET_VALUE_TYPE(value_ptr) (((struct Value *) value_ptr)->value_type)

	#define VALUE_AS_INT(value_ptr) 	(((struct Value *) value_ptr)->value.integer_val)
	#define VALUE_AS_LONG(value_ptr) 	(((struct Value *) value_ptr)->value.long_val)
	#define VALUE_AS_DOUBLE(value_ptr) 	(((struct Value *) value_ptr)->value.double_val)
	#define VALUE_AS_STRING(value_ptr) 	(((struct Value *) value_ptr)->value.pointer)
	#define VALUE_AS_PTR(value_ptr) 	(((struct Value *) value_ptr)->value.pointer)

	// void v_print_value(const void *value);
	// void *v_copy_value(const void *value);

	// bool v_is_value_true(const void *value);
	// bool v_are_values_equal(const void *value1, const void *value2);

	// void *v_add_values(const void *value1, const void *value2);
	// void *v_sub_values(const void *value1, const void *value2);
	// void *v_mul_values(const void *value1, const void *value2);
	// void *v_div_values(const void *value1, const void *value2);
	// void *v_mod_values(const void *value1, const void *value2);

#endif // value_h
