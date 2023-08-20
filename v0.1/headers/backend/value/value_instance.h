#if !defined(value_instance_h)
	#define value_instance_h

	#include <stdint.h>

	// runtime representation of an instance of a class
	struct InstanceValue {
		const void *type;

		const void *klass;

		// map of names to properties
		void *properties;

		uint32_t reference_count;
	};

	extern const void *InstanceValue;

	#define IS_INSTANCE_VALUE(ptr) ((ptr) && (((struct InstanceValue *) ptr)->type == InstanceValue))

	const void *instance_get_property(const void *instance, const char *property_name);
	const void *instance_set_property(const void *instance, const char *property_name, const void *value);

#endif // value_instance_h
