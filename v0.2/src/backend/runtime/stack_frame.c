#include "stack_frame.h"

#include <assert.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "type.h"
#include "object.h"
#include "hash_table.h"
#include "value.h"

#include "def_type.h"
#include "symbol.h"
#include "symbol_table.h"

#include "values.h"



static void *stack_frame_constructor(void *self, va_list *ap);
static void *stack_frame_destructor(void *self);

static const struct Type _type_stack_frame = {
	.size = sizeof(struct StackFrame),
	.constructor = stack_frame_constructor,
	.destructor = stack_frame_destructor
};

const void *StackFrame = &_type_stack_frame;



// v0.1
// ... = new(StackFrame, symbol_table, linked_frame)

/* static void *stack_frame_constructor(void *_self, va_list *ap) {
	struct StackFrame *self = _self;
	assert(IS_STACK_FRAME(self));

	const void *symbol_table = va_arg(*ap, const void *);
	if(NULL != symbol_table) {
		assert(IS_SYMBOL_TABLE(symbol_table));
	}
	self->nesting_level = SYMBOL_TABLE_GET_NESTING_LEVEL(symbol_table);

	self->nesting_level = va_arg(*ap, int);
	self->linked_frame = va_arg(*ap, const void *);

	if(NULL != self->linked_frame) {
		assert(IS_STACK_FRAME(self->linked_frame));
	}

	self->memory_map = new(HashTable);

	if(NULL != symbol_table) {
		struct Entry *entries = symbol_table_get_symbols(symbol_table);

		if(NULL != entries) {
			for(uint32_t i = 0; NULL != (entries + i)->key; i++) {
				const void *_symbol = (entries + i)->value;
				const struct Symbol *symbol = _symbol;

				if(symbol->definition_type == DEF_TYPE_VAR || symbol->definition_type == DEF_TYPE_VAL) {
					void *value = new(Value, VALUE_TYPE_INTEGER, 0);
					hashtable_put(self->memory_map, (entries + i)->key, value);
				}
			}
			free(entries);
		}
	}

	return _self;
}
*/


// v0.2
// ... = new(StackFrame, nesting_level, linked_frame)
static void *stack_frame_constructor(void *_self, va_list *ap) {
	#if defined(DEBUG)
		vi_log("stack_frame_constructor(self: %x)\n", _self);
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
		vi_log("ret stack_frame_constructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}



// v0.3
// ... = new(StackFrame, frame_id, nesting_level, linked_frame)
/*
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
*/

static void *stack_frame_destructor(void *_self) {
	#if defined(DEBUG)
		vi_log("stack_frame_destructor(self: %x)\n", _self);
	#endif // DEBUG

	struct StackFrame *self = _self;
	assert(IS_STACK_FRAME(self));

	delete((void *) self->memory_map);

	#if defined(DEBUG)
		vi_log("ret stack_frame_destructor(self: %x)\n", _self);
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
	printf("    linked_frame: %x\n", self->linked_frame);
	printf("    memory_map: ");
	print_hashtable(self->memory_map);
	printf("}\n");
}
// */


void sframe_set_linked_frame(void *_self, const void *linked_frame) {
	#if defined(DEBUG)
		vi_log("sframe_set_linked_frame(self: %x, frame: %x)\n\n", _self, linked_frame);
	#endif // DEBUG

	struct StackFrame *self = _self;
	assert(IS_STACK_FRAME(self));
	assert(IS_STACK_FRAME(linked_frame));

	self->linked_frame = linked_frame;
}


void sframe_put_value(void *_self, const char *key, const void *value) {
	#if defined(DEBUG)
		vi_log("sframe_put_value(self: %x, key: '%s', value: %x)\n", _self, key, value);
	#endif // DEBUG

	struct StackFrame *self = _self;
	assert(IS_STACK_FRAME(self));
	assert(is_value(value));

	hashtable_put(self->memory_map, key, value);

	#if defined(DEBUG)
		vi_log("ret sframe_put_value(self: %x, key: '%s', value: %x)\n", _self, key, value);
	#endif // DEBUG
}

void sframe_set_value(void *_self, const char *key, const void *value) {
	#if defined(DEBUG)
		vi_log("sframe_set_value(self: %x, key: '%s', value: %x)\n", _self, key, value);
	#endif // DEBUG

	struct StackFrame *self = _self;
	assert(IS_STACK_FRAME(self));
	assert(is_value(value));

	hashtable_set(self->memory_map, key, value);

	#if defined(DEBUG)
		vi_log("ret sframe_set_value(self: %x, key: '%s', value: %x)\n", _self, key, value);
	#endif // DEBUG
}

const void *sframe_get_value(const void *_self, const char *key) {
	#if defined(DEBUG)
		vi_log("sframe_get_value(self: %x, key: '%s')\n", _self, key);
	#endif // DEBUG

	const struct StackFrame *self = _self;
	assert(IS_STACK_FRAME(self));

	const void *value = hashtable_get(self->memory_map, key);

	#if defined(DEBUG)
		vi_log("ret sframe_get_value(self: %x, key: '%s'): value at %x\n", _self, key, value);
	#endif // DEBUG

	return value;
}


