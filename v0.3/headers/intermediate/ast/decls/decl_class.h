#if !defined(decl_class_h)
    #define decl_class_h

    #include <stdint.h>

    struct ClassDecl {
        const void *type;

        const void *symbol;
        void *symbol_table;

        const void *super_class_symbol;

        void *constr_decl;

        void **methods;
    };

    extern const void *ClassDecl;

    #define IS_CLASS_DECL(ptr) ((ptr) && (((const struct ClassDecl *) ptr)->type == ClassDecl))

#endif // decl_class_h

