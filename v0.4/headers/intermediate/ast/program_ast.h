#if !defined(program_ast_h)
    #define program_ast_h

    #include <stdbool.h>
    #include <stdint.h>

    struct ProgramAst {
        const void *type;

        char *program_name;
        void *symbol_table;

        uint64_t imports_count;
        void **imports;

        uint64_t decls_count;
        void **declarations;
    };

    extern const void *ProgramAst;

    #define IS_PROGRAM_AST(ptr) ((ptr) && (((const struct ProgramAst *) ptr)->type == ProgramAst))
    
    #define PROGRAM_AST_PROGRAM_NAME(ptr) (((const struct ProgramAst *) ptr)->program_name)
    #define PROGRAM_AST_SYMBOL_TABLE(ptr) (((const struct ProgramAst *) ptr)->symbol_table)

#endif // program_ast_h
