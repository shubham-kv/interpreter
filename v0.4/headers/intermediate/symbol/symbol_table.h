#if !defined(symbol_table_h)
    #define symbol_table_h

    #include "stdbool.h"
    #include "stdint.h"
    #include "hash_table.h"

    struct SymbolTable {
        const void *type;

        uint8_t nesting_level;

        void *entries;
    };

    extern const void *SymbolTable;

    #define IS_SYMBOL_TABLE(ptr) ((ptr) && (((const struct SymbolTable *) ptr)->type == SymbolTable))

    #define SYMBOL_TABLE_GET_NESTING_LEVEL(ptr) (((const struct SymbolTable *) ptr)->nesting_level)
    #define SYMBOL_TABLE_ENTRIES(ptr) (((const struct SymbolTable *) ptr)->entries)

    void symbol_table_define(const void *symbol_table, const char *id, const void *symbol);
    const void *symbol_table_lookup(const void *symbol_table, const char *id);
    
    void symbol_table_copy(void *symbol_table_1, const void *symbol_table_2);
    
    struct Entry *symbol_table_get_symbols(const void *symbol_table);

#endif // symbol_table_h
