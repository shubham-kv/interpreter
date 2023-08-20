#include "symbol_table.h"

#include <assert.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "hash_table.h"
#include "type.h"
#include "object.h"

#include "symbol.h"

#include "log.h"


static void *symbol_table_constructor(void *self, va_list *ap);
static void *symbol_table_destructor(void *self);


static const struct Type _type_symbol_table = {
    sizeof(struct SymbolTable),
    symbol_table_constructor,
    symbol_table_destructor
};

const void *SymbolTable = &_type_symbol_table;


// ... = new(SymbolTable, 0);
// ... = new(SymbolTable, nesting_level);

static void *symbol_table_constructor(void *_self, va_list *ap) {
    #if defined(DEBUG)
        vi_log("symbol_table_constructor(self: %x)\n", _self);
    #endif // DEBUG

    struct SymbolTable *self = _self;
    assert(IS_SYMBOL_TABLE(self));

    self->nesting_level = (uint8_t) va_arg(*ap, int);
    self->entries = new(HashTable);

    #if defined(DEBUG)
        vi_log("ret symbol_table_constructor(self: %x)\n", _self);
    #endif // DEBUG

    return _self;
}

static void *symbol_table_destructor(void *_self) {
    #if defined(DEBUG)
        vi_log("symbol_table_destructor(self: %x)\n", _self);
    #endif // DEBUG

    struct SymbolTable *self = _self;
    assert(IS_SYMBOL_TABLE(self));

    delete((void *) self->entries);

    #if defined(DEBUG)
        vi_log("ret symbol_table_destructor(self: %x)\n", _self);
    #endif // DEBUG

    return _self;
}


void symbol_table_define(const void *_self, const char *id, const void *symbol) {

    #if defined(DEBUG)
        vi_log("symbol_table_define(self: %x, id: '%s', symbol: %x)\n", _self, id, symbol);
    #endif // DEBUG

    const struct SymbolTable *self = _self;
    assert(IS_SYMBOL_TABLE(self));
    assert(IS_SYMBOL(symbol));
    assert(id);

    hashtable_put(self->entries, id, symbol);

    #if defined(DEBUG)
        vi_log("ret symbol_table_define(self: %x, id: '%s', symbol: %x)\n\n", _self, id, symbol);
    #endif // DEBUG

    return;
}


const void *symbol_table_lookup(const void *_self, const char *id) {
    #if defined(DEBUG)
        vi_log("symbol_table_lookup(self: %x, id: '%s')\n", _self, id);
    #endif // DEBUG

    const struct SymbolTable *self = _self;
    assert(IS_SYMBOL_TABLE(self));
    assert(id);

    const void *symbol = (void *) hashtable_get(self->entries, id);

    #if defined(DEBUG)
        vi_log("ret symbol_table_lookup(self: %x, id: '%s'): symbol at %x\n\n", _self, id, symbol);
    #endif // DEBUG

    return symbol;
}


void symbol_table_copy(void *_self, const void *_other) {
    struct SymbolTable *self = _self;
    assert(IS_SYMBOL_TABLE(self));

    const struct SymbolTable *other = _other;
    assert(IS_SYMBOL_TABLE(other));

    hashtable_copy(self->entries, other->entries);
}




struct Entry *symbol_table_get_symbols(const void *_self) {
    const struct SymbolTable *self = _self;
    assert(IS_SYMBOL_TABLE(self));

    return hashtable_get_entries(self->entries);
}


