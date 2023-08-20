#if !defined(value_object_h)
	#define value_object_h

	#include <stdbool.h>

	bool is_value(const void *value);

	void print_value(const void *value);

	void *copy_value(const void *value);

	bool is_value_true(const void *value);

	bool are_values_equal(const void *value, const void *other);

	void *add_values(const void *value, const void *other);

	void *sub_values(const void *value, const void *other);
	
	void *mul_values(const void *value, const void *other);

	void *div_values(const void *value, const void *other);
	
	void *mod_values(const void *value, const void *other);

#endif // value_object_h

