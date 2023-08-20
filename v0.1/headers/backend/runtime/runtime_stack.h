#if !defined(runtime_stack_h)
	#define runtime_stack_h

	#include <stdint.h>

	struct RuntimeStack {
		const void *type;

		uint32_t stack_size;

		void **stack;

		void *frame_pointer;
	};

	extern const void *RuntimeStack;

	#define IS_RUNTIME_STACK(ptr) ((ptr) && ((RuntimeStack) == ((struct RuntimeStack *) ptr)->type))


	const void *rstack_push(void *runtime_stack, const void *symbol_table);
	void rstack_pop(void *runtime_stack);

	const void *rstack_get_local_frame(const void *runtime_stack);

	const void *rstack_get_value(const void *runtime_stack, const void *symbol);
	void rstack_set_value(const void *runtime_stack, const void *symbol, const void *value);

#endif // runtime_stack_h
