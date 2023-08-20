#if !defined(frame_pointer_h)
	#define frame_pointer_h

	#include <stdint.h>

	struct FramePointer {
		const void *type;

		// max no. of pointers it can hold
		uint32_t pointers_size;

		// number of pointers now
		uint32_t count;

		// the array of pointers
		void **pointers;
	};

	extern const void *FramePointer;


	#define IS_FRAME_POINTER(ptr) ((ptr) && ((FramePointer) == ((struct FramePointer *) ptr)->type))

	#define FP_GET_COUNT(frame_pointer) (((struct FramePointer *) frame_pointer)->count)


	// uint32_t fp_get_count(const void *frame_pointer);
	const void *fp_get_topmost_frame(const void *frame_pointer, uint32_t nesting_level);

	void fp_enter_block_update(void *frame_pointer, void *stack_frame);
	void fp_exit_block_update(void *frame_pointer, uint32_t nesting_level);

#endif // frame_pointer_h
