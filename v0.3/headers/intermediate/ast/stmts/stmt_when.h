#if !defined(stmt_when_h)
    #define stmt_when_h

    struct WhenCase {
        const void *type;

        // array of exprs to this case
        void **exprs;

        // the stmt to execute if any of the exprs match
        void *stmt;
    };

    extern const void *WhenCase;
    #define IS_WHEN_CASE(ptr) ((ptr) && (((const struct WhenCase *) ptr)->type == WhenCase))


    struct WhenStmt {
        const void *type;

        // the when's condition expr
        void *expr;

        // the array of pointers to cases
        struct WhenCase **cases;

        // the statement to execute when none of the cases are matched
        void *else_stmt;
    };

    extern const void *WhenStmt;
    #define IS_WHEN_STMT(ptr) ((ptr) && (((const struct WhenStmt *) ptr)->type == WhenStmt))

#endif // stmt_when_h
