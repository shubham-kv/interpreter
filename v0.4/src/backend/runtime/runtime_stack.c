#include "runtime_stack.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_table.h"
#include "my_memory.h"
#include "object.h"
#include "type.h"

#include "headers_backend_runtime.h"
#include "headers_intermediate_symbol.h"

#include "log.h"


#define MAX_STACK_DEPTH 1024


static void *runtime_stack_constructor(void *self, va_list *ap);
static void *runtime_stack_destructor(void *self);

static const struct Type _type_runtime_stack = {
    .size = sizeof(struct RuntimeStack),
    .constructor = runtime_stack_constructor,
    .destructor = runtime_stack_destructor
};

const void *RuntimeStack = &_type_runtime_stack;



static void *runtime_stack_constructor(void *_self, va_list *ap) {
    #if defined(DEBUG)
        vi_log("runtime_stack_constructor(self: %#x)\n", _self);
    #endif // DEBUG

    struct RuntimeStack *self = _self;
    assert(IS_RUNTIME_STACK(self));

    self->stack_size = 0;
    self->stack = NULL;

    self->frame_pointer = new(FramePointer);

    #if defined(DEBUG)
        vi_log("ret runtime_stack_constructor(self: %#x)\n", _self);
    #endif // DEBUG

    return _self;
}

static void *runtime_stack_destructor(void *_self) {
    #if defined(DEBUG)
        vi_log("runtime_stack_destructor(self: %#x)\n", _self);
    #endif // DEBUG

    struct RuntimeStack *self = _self;
    assert(IS_RUNTIME_STACK(self));

    delete(self->frame_pointer);
    free(self->stack);

    #if defined(DEBUG)
        vi_log("ret runtime_stack_destructor(self: %#x)\n", _self);
    #endif // DEBUG

    return _self;
}



const void *rstack_push(void *_self, int nesting_level) {
    #if defined(DEBUG)
        vi_log("rstack_push(self: %#x, nesting_level: %d)\n", _self, nesting_level);
    #endif // DEBUG
    
    struct RuntimeStack *self = _self;
    assert(IS_RUNTIME_STACK(self));

    if(self->stack_size >= MAX_STACK_DEPTH) {
        fprintf(stderr, "RuntimeError: Maximum stack depth reached.");
		exit(EXIT_FAILURE);
    }

    void *new_frame = new(StackFrame, nesting_level, NULL);

    self->stack = reallocate(self->stack, sizeof(void *), ++self->stack_size);
    self->stack[self->stack_size - 1] = new_frame;

    fp_enter_block_update(self->frame_pointer, new_frame);

    #if defined(DEBUG)
        vi_log("ret rstack_push(self: %#x, nesting_level: %d)\n\n", _self, nesting_level);
    #endif // DEBUG

    return new_frame;
}

const void *rstack_push_frame(void *_self, void *stack_frame) {
    #if defined(DEBUG)
        vi_log("rstack_push_frame(self: %#x, stack_frame: %#x)\n", _self, stack_frame);
    #endif // DEBUG
    
    struct RuntimeStack *self = _self;
    assert(IS_RUNTIME_STACK(self));
    assert(IS_STACK_FRAME(stack_frame));

    if(self->stack_size >= MAX_STACK_DEPTH) {
		fprintf(stderr, "RuntimeError: Maximum stack depth reached.");
		exit(EXIT_FAILURE);
    }

    self->stack = reallocate(self->stack, sizeof(void *), ++self->stack_size);
    self->stack[self->stack_size - 1] = stack_frame;

    fp_enter_block_update(self->frame_pointer, stack_frame);

    #if defined(DEBUG)
        vi_log("ret rstack_push_frame(self: %#x, stack_frame: %#x)\n", _self, stack_frame);
    #endif // DEBUG

    return stack_frame;
}


void rstack_pop(void *_self) {
    #if defined(DEBUG)
        vi_log("rstack_pop(self: %#x)\n", _self);
    #endif // DEBUG

    struct RuntimeStack *self = _self;
    assert(IS_RUNTIME_STACK(self));

    if(self->stack_size < 1) return;

    const void *top_frame = self->stack[self->stack_size - 1];
    fp_exit_block_update(self->frame_pointer, STACK_FRAME_NESTING_LEVEL(top_frame));

    delete((void *) top_frame);

    self->stack[self->stack_size - 1] = NULL;
    self->stack = reallocate(self->stack, sizeof(void *), --self->stack_size);

    #if defined(DEBUG)
        vi_log("ret rstack_pop(self: %#x)\n\n", _self);
    #endif // DEBUG
}

const void *rstack_get_local_frame(const void *_self) {
    #if defined(DEBUG)
        vi_log("rstack_get_local_frame(self: %#x)\n", _self);
    #endif // DEBUG

    const struct RuntimeStack *self = _self;
    assert(IS_RUNTIME_STACK(self));

    const void *value = self->stack[self->stack_size - 1];

    #if defined(DEBUG)
        vi_log("ret rstack_get_local_frame(self: %#x): value at %#x\n\n", _self, value);
    #endif // DEBUG

    return value;
}


void rstack_put_value(const void *_self, const void *symbol, const void *value) {
    #if defined(DEBUG)
        vi_log("rstack_put_value(self: %#x, symbol: %#x, value: %#x)\n", _self, symbol, value);
    #endif // DEBUG

    const struct RuntimeStack *self = _self;
    assert(IS_RUNTIME_STACK(self));

    uint32_t nesting_level = symbol_get_nesting_level(symbol);

    const void *frame = fp_get_topmost_frame(self->frame_pointer, nesting_level);
    const char *key = SYMBOL_GET_ID(symbol);

    sframe_put_value((void *) frame, key, value);

    #if defined(DEBUG)
        vi_log("ret rstack_put_value(self: %#x, symbol_id: '%s', value: %#x)\n\n", _self, key, value);
    #endif // DEBUG
}


void rstack_set_value(const void *_self, const void *symbol, const void *value) {
    #if defined(DEBUG)
        vi_log("rstack_set_value(self: %#x, symbol: %#x, value: %#x)\n", _self, symbol, value);
    #endif // DEBUG

    const struct RuntimeStack *self = _self;
    assert(IS_RUNTIME_STACK(self));

    uint32_t nesting_level = symbol_get_nesting_level(symbol);

    const void *frame = fp_get_topmost_frame(self->frame_pointer, nesting_level);
    const char *key = SYMBOL_GET_ID(symbol);

    sframe_set_value((void *) frame, key, value);

    #if defined(DEBUG)
        vi_log("ret rstack_set_value(self: %#x, symbol_id: '%s', value: %#x)\n\n", _self, key, value);
    #endif // DEBUG    
}


const void *rstack_get_value(const void *_self, const void *symbol) {
    #if defined(DEBUG)
        vi_log("rstack_get_value(self: %#x, symbol: %#x)\n", _self, symbol);
    #endif // DEBUG

    const struct RuntimeStack *self = _self;
    assert(IS_RUNTIME_STACK(self));

    uint32_t nesting_level = symbol_get_nesting_level(symbol);
    
    const void *frame = fp_get_topmost_frame(self->frame_pointer, nesting_level);
    const char *key = SYMBOL_GET_ID(symbol);

    const void *value = sframe_get_value(frame, key);

    #if defined(DEBUG)
        vi_log("ret rstack_get_value(self: %#x, symbol_id: '%s'): value at %#x\n\n", _self, key, value);
    #endif // DEBUG

    return value;
}

