#if !defined(stack_frame_h)
	#define stack_frame_h

	#include <stdbool.h>
	#include <stdint.h>

	struct StackFrame {
		const void *type;

		uint32_t nesting_level;

		const void *linked_frame;

		// hashtable of names to runtime values
		void *memory_map;
	};

	extern const void *StackFrame;

	#define IS_STACK_FRAME(ptr) ((ptr) && ((StackFrame) == ((struct StackFrame *) ptr)->type))

	#define STACK_FRAME_GET_NESTING_LEVEL(stack_frame) 	(((struct StackFrame *) stack_frame)->nesting_level)
	#define STACK_FRAME_GET_LINKED_FRAME(stack_frame) 	(((struct StackFrame *) stack_frame)->linked_frame)


	void print_stack_frame(const void *stack_frame);


	void sframe_set_linked_frame(void *stack_frame, const void *linked_frame);

	void sframe_put_value(void *stack_frame, const char *key, const void *value);
	const void *sframe_get_value(const void *stack_frame, const char *key);
	void sframe_set_value(void *stack_frame, const char *key, const void *value);

#endif // stack_frame_h
