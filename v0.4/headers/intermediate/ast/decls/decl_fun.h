#if !defined(decl_fun_h)
    #define decl_fun_h

    #include <stdbool.h>
    #include <stdint.h>

    struct FunDecl {
        const void *type;

        // fun declaration symbol in it's parent's symbol table
        const void *symbol;

        // fun's symbol table
        void *symbol_table;

        uint32_t param_count;

        // list of parameter symbols defined in fun's symbol table
        void **param_symbols;

        // list of statements to execute
        void **declarations;
    };

    extern const void *FunDecl;

    #define IS_FUN_DECL(ptr) ((ptr) && (((const struct FunDecl *) ptr)->type == FunDecl))

    #define FUN_DECL_GET_SYMBOL(ptr)        (((const struct FunDecl *) ptr)->symbol)
    #define FUN_DECL_GET_SYMBOL_TABLE(ptr)  (((const struct FunDecl *) ptr)->symbol_table)
    #define FUN_DECL_GET_PARAM_COUNT(ptr)   (((const struct FunDecl *) ptr)->param_count)
    #define FUN_DECL_GET_PARAM_SYMBOLS(ptr) (((const struct FunDecl *) ptr)->param_symbols)
    #define FUN_DECL_GET_DECLARATIONS(ptr)  (((const struct FunDecl *) ptr)->declarations)

    #define MAX_PARAM_COUNT (256)

#endif // decl_fun_h
