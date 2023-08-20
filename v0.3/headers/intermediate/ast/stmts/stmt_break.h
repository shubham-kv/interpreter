#if !defined(stmt_break_h)
#define stmt_break_h

    #include "token.h"

    struct BreakStmt {
        const void *type;

        const struct Token *break_token;

        const struct Token *semicolon_token;
    };

    extern const void *BreakStmt;

    #define IS_BREAK_STMT(ptr) ((ptr) && (((const struct BreakStmt *) ptr)->type == BreakStmt))

#endif // stmt_break_h

