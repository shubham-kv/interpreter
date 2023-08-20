#include "runtime_stack.h"

#include <assert.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "object.h"
#include "hash_table.h"
#include "memory.h"

#include "runtime_err_handler.h"
#include "stack_frame.h"
#include "frame_pointer.h"

#include "symbol.h"
#include "symbol_table.h"


#define MAX_STACK_DEPTH 1024


static void *runtime_stack_constructor(void *self, va_list *ap);
static void *runtime_stack_destructor(void *self);

static const struct Type _type_runtime_stack = {
	sizeof(struct RuntimeStack),
	runtime_stack_constructor,
	runtime_stack_destructor
};

const void *RuntimeStack = &_type_runtime_stack;

static void *runtime_stack_constructor(void *_self, va_list *ap) {
	struct RuntimeStack *self = _self;
	assert(IS_RUNTIME_STACK(self));

	self->stack_size = 0;
	self->stack = NULL;

	self->frame_pointer = new(FramePointer);

	return _self;
}

static void *runtime_stack_destructor(void *_self) {
	struct RuntimeStack *self = _self;
	assert(IS_RUNTIME_STACK(self));
	assert(self->stack_size == 0);

	delete(self->frame_pointer);
	free(self->stack);

	return _self;
}



const void *rstack_push(void *_self, const void *symbol_table) {
	struct RuntimeStack *self = _self;
	assert(IS_RUNTIME_STACK(self));

	if(self->stack_size >= MAX_STACK_DEPTH) {
		runtime_error(
			"RuntimeError: Maximum stack depth reached."
		);
	}

	void *new_frame = new(StackFrame, symbol_table, NULL);

	self->stack = reallocate(self->stack, sizeof(void *), ++self->stack_size);
	self->stack[self->stack_size - 1] = new_frame;
	
	fp_enter_block_update(self->frame_pointer, new_frame);

	return new_frame;
}


void rstack_pop(void *_self) {
	struct RuntimeStack *self = _self;
	assert(IS_RUNTIME_STACK(self));

	if(self->stack_size < 1) return;

	const void *top_frame = self->stack[self->stack_size - 1];
	// uint32_t nesting_level = sframe_get_nesting_level(top_frame);

	fp_exit_block_update(self->frame_pointer, STACK_FRAME_GET_NESTING_LEVEL(top_frame));

	delete((void *) top_frame);

	self->stack[self->stack_size - 1] = NULL;
	self->stack = reallocate(self->stack, sizeof(void *), --self->stack_size);
}

const void *rstack_get_local_frame(const void *_self) {
	const struct RuntimeStack *self = _self;
	assert(IS_RUNTIME_STACK(self));

	return self->stack[self->stack_size - 1];
}




const void *rstack_get_value(const void *_self, const void *symbol) {
	const struct RuntimeStack *self = _self;
	assert(IS_RUNTIME_STACK(self));

	uint32_t nesting_level = symbol_get_nesting_level(symbol);
	
	const void *frame = fp_get_topmost_frame(self->frame_pointer, nesting_level);
	const char *key = SYMBOL_GET_ID(symbol);

	return sframe_get_value(frame, key);
}


void rstack_set_value(const void *_self, const void *symbol, const void *value) {
	const struct RuntimeStack *self = _self;
	assert(IS_RUNTIME_STACK(self));

	uint32_t nesting_level = symbol_get_nesting_level(symbol);

	const void *frame = fp_get_topmost_frame(self->frame_pointer, nesting_level);
	const char *key = SYMBOL_GET_ID(symbol);

	sframe_set_value((void *) frame, key, value);
}



