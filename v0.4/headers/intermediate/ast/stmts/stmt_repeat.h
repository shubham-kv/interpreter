#if !defined(stmt_repeat_h)
    #define stmt_repeat_h

    #include "token.h"

    struct RepeatStmt {
        const void *type;

        void *symbol_table;

        const struct Token *id_token;

        void *expr;

        void **child_stmts;
    };

    extern const void *RepeatStmt;

    #define IS_REPEAT_STMT(ptr) ((ptr) && (((const struct RepeatStmt *) ptr)->type == RepeatStmt))

#endif // stmt_repeat_h
