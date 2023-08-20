#if !defined(stmt_loop_h)
    #define stmt_loop_h

    #include "token.h"

    enum LoopStmtType {
        LOOP_STMT_TYPE_NONE,
        LOOP_STMT_TYPE_CONDITIONAL,
        LOOP_STMT_TYPE_RANGED
    };

    struct LoopStmt {
        const void *type;

        const void *symbol_table;

        enum LoopStmtType loop_type;

        const struct Token *id_token;

        void *start_expr;

        const struct Token *arrow_token;

        void *end_expr;

        void *step_expr;

        void **child_stmts;
    };

    extern const void *LoopStmt;

    #define IS_LOOP_STMT(ptr) ((ptr) && (((const struct LoopStmt *) ptr)->type == LoopStmt))

#endif // stmt_loop_h
