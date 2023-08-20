#if !defined(value_mib_h)
#define value_mib_h

	#include <stdint.h>

	// MIB -> Method Instance Binding

	struct MibValue {
		const void *type;

		// the funValue
		const void *method;

		// the instanceValue to which the method is bound to
		const void *instance;

		uint32_t reference_count;
	};

	extern const void *MibValue;

	#define IS_MIB_VALUE(ptr) ((ptr) && (((struct MibValue *) ptr)->type == MibValue))

#endif // value_mib_h
