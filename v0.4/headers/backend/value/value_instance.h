#if !defined(value_instance_h)
	#define value_instance_h

	#include <stdint.h>

	// runtime representation of an instance of a class
	struct InstanceValue {
		const void *type;

		const void *klass;

		// map of names to properties
		void *fields;

		uint32_t reference_count;
	};

	extern const void *InstanceValue;

	#define IS_INSTANCE_VALUE(ptr) ((ptr) && (((struct InstanceValue *) ptr)->type == InstanceValue))

	void instance_put_field(void *_self, const char *name, const void *value);
	const void *instance_get_field(const void *_self, const char *name);
	// void *instance_get_method(const void *instance, const char *name);

	// looks for methods pertaining to the corresponding klass
	void *instance_get_method(const void *_self, const void *_klass, const char *name);

#endif // value_instance_h
