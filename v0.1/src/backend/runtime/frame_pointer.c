#include "frame_pointer.h"

#include <assert.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
	sizeof(struct FramePointer),
	frame_pointer_constructor,
	frame_pointer_destructor
};

const void *FramePointer = &_type_frame_pointer;

static void *frame_pointer_constructor(void *_self, va_list *ap) {
	struct FramePointer *self = _self;
	assert(IS_FRAME_POINTER(self));

	self->count = 0;
	self->pointers_size = FP_POINTERS_STEP_SIZE;
	self->pointers = allocate(sizeof(void *), self->pointers_size);

	return _self;
}

static void *frame_pointer_destructor(void *_self) {
	struct FramePointer *self = _self;
	assert(IS_FRAME_POINTER(self));

	free(self->pointers);

	return _self;
}

// uint32_t fp_get_count(const void *_self) {
// 	const struct FramePointer *self = _self;
// 	assert(IS_FRAME_POINTER(self));
// 	return self->count;
// }

const void *fp_get_topmost_frame(const void *_self, uint32_t nesting_level) {
	const struct FramePointer *self = _self;
	assert(IS_FRAME_POINTER(self));
	assert(nesting_level < self->count);

	return self->pointers[nesting_level];
}



void fp_enter_block_update(void *_self, void *stack_frame) {
	struct FramePointer *self = _self;
	assert(IS_FRAME_POINTER(self));
	assert(IS_STACK_FRAME(stack_frame));

	if(self->count == self->pointers_size) {
		self->pointers_size += FP_POINTERS_STEP_SIZE;
		self->pointers = reallocate(self->pointers, sizeof(void *), self->pointers_size);
	}

	// printf("entering frame: %x\n", stack_frame);

	uint32_t nesting_level = STACK_FRAME_GET_NESTING_LEVEL(stack_frame);
	// printf("nesting_level: %d, self->count: %d\n", nesting_level, self->count);

	if(nesting_level >= self->count) {
		self->pointers[self->count++] = stack_frame;
		// printf("ifblock nlevel: %d, self->count: %d\n\n", nesting_level, self->count);
	}
	else {
		const void *toppest_frame = self->pointers[nesting_level];

		sframe_set_linked_frame(stack_frame, toppest_frame);
		self->pointers[nesting_level] = stack_frame;
		
		// printf("eblock nlevel: %d, self->count: %d\n\n", nesting_level, self->count);
	}
}


void fp_exit_block_update(void *_self, uint32_t nesting_level) {
	struct FramePointer *self = _self;
	assert(IS_FRAME_POINTER(self));
	assert(nesting_level < self->count);

	const void *frame = self->pointers[nesting_level];
	// printf("exiting frame: %x\n", frame);

	void *linked_frame = (void *) STACK_FRAME_GET_LINKED_FRAME(frame);

	self->pointers[nesting_level] = linked_frame;

	if(NULL == linked_frame)
		self->count--;
}

#undef FP_POINTERS_STEP_SIZE
