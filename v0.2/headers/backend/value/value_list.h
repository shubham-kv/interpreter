#if !defined(value_list_h)
	#define value_list_h

	#include <stdint.h>
	#include "value_type.h"

	struct ListValue {
		const void *type;

		// enum ValueType value_type;

		// the total count of elements the list has
		int count;
		void **values;

		// the no. of references to this value
		uint64_t reference_count;
	};

	extern const void *ListValue;

	#define IS_LIST_VALUE(ptr) ((ptr) && (((struct ListValue *) ptr)->type == ListValue))

	void print_list_value(const void *list_value);


#endif // value_list_h
