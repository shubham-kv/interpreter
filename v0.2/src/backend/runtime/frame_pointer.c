#include "frame_pointer.h"

#include <assert.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "log.h"
#include "type.h"
#include "object.h"
#include "hash_table.h"
#include "memory.h"

#include "symbol_table.h"

#include "stack_frame.h"


#define FP_POINTERS_STEP_SIZE 8


static void *frame_pointer_constructor(void *self, va_list *ap);
static void *frame_pointer_destructor(void *self);

static const struct Type _type_frame_pointer = {
	.size = sizeof(struct FramePointer),
	.constructor = frame_pointer_constructor,
	.destructor = frame_pointer_destructor
};

const void *FramePointer = &_type_frame_pointer;

static void *frame_pointer_constructor(void *_self, va_list *ap) {
	#if defined(DEBUG)
		vi_log("frame_pointer_constructor(self: %x)\n", _self);
	#endif // DEBUG

	struct FramePointer *self = _self;
	assert(IS_FRAME_POINTER(self));

	self->count = 0;
	self->pointers_size = FP_POINTERS_STEP_SIZE;
	self->pointers = allocate(sizeof(void *), self->pointers_size);

	#if defined(DEBUG)
		vi_log("ret frame_pointer_constructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}

static void *frame_pointer_destructor(void *_self) {
	#if defined(DEBUG)
		vi_log("frame_pointer_destructor(self: %x)\n", _self);
	#endif // DEBUG

	struct FramePointer *self = _self;
	assert(IS_FRAME_POINTER(self));

	free(self->pointers);

	#if defined(DEBUG)
		vi_log("ret frame_pointer_destructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}


const void *fp_get_topmost_frame(const void *_self, uint32_t nesting_level) {
	#if defined(DEBUG)
		vi_log("fp_get_topmost_frame(self: %x, nesting_level: %d)\n", _self, nesting_level);
	#endif // DEBUG

	const struct FramePointer *self = _self;
	assert(IS_FRAME_POINTER(self));
	assert(nesting_level < self->count);
	assert(IS_STACK_FRAME(self->pointers[nesting_level]));

	const void *stack_frame = self->pointers[nesting_level];

	#if defined(DEBUG)
		vi_log("ret fp_get_topmost_frame(self: %x, nesting_level: %d): frame at %x\n", _self, nesting_level, stack_frame);
	#endif // DEBUG

	return stack_frame;
}



void fp_enter_block_update(void *_self, void *stack_frame) {
	#if defined(DEBUG)
		vi_log("fp_enter_block_update(self: %x, frame: %x)\n", _self, stack_frame);
	#endif // DEBUG

	struct FramePointer *self = _self;
	assert(IS_FRAME_POINTER(self));
	assert(IS_STACK_FRAME(stack_frame));

	if(self->count == self->pointers_size) {
		self->pointers_size += FP_POINTERS_STEP_SIZE;
		self->pointers = reallocate(self->pointers, sizeof(void *), self->pointers_size);
	}

	uint32_t nesting_level = STACK_FRAME_GET_NESTING_LEVEL(stack_frame);

	if(nesting_level >= self->count) {
		self->pointers[self->count++] = stack_frame;
	}
	else {
		const void *toppest_frame = self->pointers[nesting_level];

		sframe_set_linked_frame(stack_frame, toppest_frame);
		self->pointers[nesting_level] = stack_frame;
	}

	#if defined(DEBUG)
		vi_log("ret fp_enter_block_update(self: %x, frame: %x)\n", _self, stack_frame);
	#endif // DEBUG
}


void fp_exit_block_update(void *_self, uint32_t nesting_level) {
	#if defined(DEBUG)
		vi_log("fp_exit_block_update(self: %x, nesting_level: %d)\n", _self, nesting_level);
	#endif // DEBUG

	struct FramePointer *self = _self;
	assert(IS_FRAME_POINTER(self));
	assert(nesting_level < self->count);

	const void *frame = self->pointers[nesting_level];
	void *linked_frame = (void *) STACK_FRAME_GET_LINKED_FRAME(frame);

	self->pointers[nesting_level] = linked_frame;

	if(NULL == linked_frame)
		self->count--;

	#if defined(DEBUG)
		vi_log("ret fp_exit_block_update(self: %x, nesting_level: %d)\n", _self, nesting_level);
	#endif // DEBUG
}

#undef FP_POINTERS_STEP_SIZE
