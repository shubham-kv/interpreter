#include "stack_frame.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_table.h"
#include "object.h"
#include "type.h"

#include "value_object.h"

#include "log.h"



static void *stack_frame_constructor(void *self, va_list *ap);
static void *stack_frame_destructor(void *self);

static const struct Type _type_stack_frame = {
    .size = sizeof(struct StackFrame),
    .constructor = stack_frame_constructor,
    .destructor = stack_frame_destructor
};

const void *StackFrame = &_type_stack_frame;


// ... = new(StackFrame, nesting_level, linked_frame)
static void *stack_frame_constructor(void *_self, va_list *ap) {
    #if defined(DEBUG)
        vi_log("stack_frame_constructor(self: %#x)\n", _self);
    #endif // DEBUG

    struct StackFrame *self = _self;
    assert(IS_STACK_FRAME(self));

    self->nesting_level = va_arg(*ap, int);
    self->linked_frame = va_arg(*ap, const void *);

    if(NULL != self->linked_frame) {
        assert(IS_STACK_FRAME(self->linked_frame));
    }

    self->memory_map = new(HashTable);

    #if defined(DEBUG)
        vi_log("ret stack_frame_constructor(self: %#x)\n", _self);
    #endif // DEBUG

    return _self;
}

/*
// v0.3
// ... = new(StackFrame, frame_id, nesting_level, linked_frame)
static void *stack_frame_constructor(void *_self, va_list *ap) {
    struct StackFrame *self = _self;
    assert(IS_STACK_FRAME(self));

    // self->frame_id = va_arg(*ap, int);
    // assert(self->frame_id >= 0);

    self->nesting_level = va_arg(*ap, int);
    assert(self->nesting_level >= 0);

    self->linked_frame = va_arg(*ap, const void *);

    if(NULL != self->linked_frame) {
        assert(IS_STACK_FRAME(self->linked_frame));
    }

    self->memory_map = new(HashTable);

    return _self;
}
// */


static void *stack_frame_destructor(void *_self) {
    #if defined(DEBUG)
        vi_log("stack_frame_destructor(self: %#x)\n", _self);
    #endif // DEBUG

    struct StackFrame *self = _self;
    assert(IS_STACK_FRAME(self));

    delete((void *) self->memory_map);

    #if defined(DEBUG)
        vi_log("ret stack_frame_destructor(self: %#x)\n", _self);
    #endif // DEBUG

    return _self;
}


/*
void print_stack_frame(const void *_self) {
    const struct StackFrame *self = _self;
    assert(IS_STACK_FRAME(self));

    printf("StackFrame {\n");
    printf("    frame_id: %d\n", self->frame_id);
    printf("    nesting_level: %d\n", self->nesting_level);
    printf("    linked_frame: %#x\n", self->linked_frame);
    printf("    memory_map: ");
    print_hashtable(self->memory_map);
    printf("}\n");
}
// */


void sframe_set_linked_frame(void *_self, const void *linked_frame) {
    #if defined(DEBUG)
        vi_log("sframe_set_linked_frame(self: %#x, frame: %#x)\n\n", _self, linked_frame);
    #endif // DEBUG

    struct StackFrame *self = _self;
    assert(IS_STACK_FRAME(self));
    assert(IS_STACK_FRAME(linked_frame));

    self->linked_frame = linked_frame;
}


void sframe_put_value(void *_self, const char *key, const void *value) {
    #if defined(DEBUG)
        vi_log("sframe_put_value(self: %#x, key: '%s', value: %#x)\n", _self, key, value);
    #endif // DEBUG

    struct StackFrame *self = _self;
    assert(IS_STACK_FRAME(self));
    assert(is_value(value));

    hashtable_put(self->memory_map, key, value);

    #if defined(DEBUG)
        vi_log("ret sframe_put_value(self: %#x, key: '%s', value: %#x)\n\n", _self, key, value);
    #endif // DEBUG
}

void sframe_set_value(void *_self, const char *key, const void *value) {
    #if defined(DEBUG)
        vi_log("sframe_set_value(self: %#x, key: '%s', value: %#x)\n", _self, key, value);
    #endif // DEBUG

    struct StackFrame *self = _self;
    assert(IS_STACK_FRAME(self));
    assert(is_value(value));

    hashtable_set(self->memory_map, key, value);

    #if defined(DEBUG)
        vi_log("ret sframe_set_value(self: %#x, key: '%s', value: %#x)\n\n", _self, key, value);
    #endif // DEBUG
}

const void *sframe_get_value(const void *_self, const char *key) {
    #if defined(DEBUG)
        vi_log("sframe_get_value(self: %#x, key: '%s')\n", _self, key);
    #endif // DEBUG

    const struct StackFrame *self = _self;
    assert(IS_STACK_FRAME(self));

    const void *value = hashtable_get(self->memory_map, key);

    #if defined(DEBUG)
        vi_log("ret sframe_get_value(self: %#x, key: '%s'): value at %#x\n\n", _self, key, value);
    #endif // DEBUG

    return value;
}


void sframe_copy(void *_self, const void *_other) {
    #if defined(DEBUG)
        vi_log("sframe_copy(self: %#x, other: %#x)\n", _self, _other);
    #endif // DEBUG

    struct StackFrame *self = _self;
    assert(IS_STACK_FRAME(self));

    const struct StackFrame *other = _other;
    assert(IS_STACK_FRAME(other));

    hashtable_copy(self->memory_map, other->memory_map);

    #if defined(DEBUG)
        vi_log("ret sframe_copy(self: %#x, other: %#x)\n", _self, _other);
    #endif // DEBUG    
}


void sframe_clear(void *_self) {
    struct StackFrame *self = _self;
    assert(IS_STACK_FRAME(self));

    hashtable_clear(self->memory_map);
}

