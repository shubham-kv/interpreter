#if !defined(decl_var_h)
    #define decl_var_h

    struct VarDecl {
        const void *type;

        void **symbols;
    };

    extern const void *VarDecl;

    #define IS_VAR_DECL(ptr) ((ptr) && (((const struct VarDecl *) ptr)->type == VarDecl))

#endif // decl_var_h
