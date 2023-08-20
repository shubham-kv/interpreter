#if !defined(stmt_import_h)
#define stmt_import_h

    #include "token.h"

    struct ImportStmt {
        const void *type;

        const struct Token *import_token;
        const struct Token *module_id;
        const struct Token *semicolon_token;

        void *saved_source;
        void *saved_lexer;
        void *saved_parser;
    };

    extern const void *ImportStmt;

    #define IS_IMPORT_STMT(ptr) ((ptr) && (((const struct ImportStmt *) ptr)->type == ImportStmt))

    #define IMPORT_STMT_MODULE_ID(ptr) (((const struct ImportStmt *) ptr)->module_id)

    #define IMPORT_STMT_SOURCE(ptr)  (((const struct ImportStmt *) ptr)->saved_source)
    #define IMPORT_STMT_LEXER(ptr)   (((const struct ImportStmt *) ptr)->saved_lexer)
    #define IMPORT_STMT_PARSER(ptr)  (((const struct ImportStmt *) ptr)->saved_parser)

#endif // stmt_import_h
