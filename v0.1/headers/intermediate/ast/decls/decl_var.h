#if !defined(decl_var_h)
	#define decl_var_h

	struct VarDecl {
		const void *type;

		void **symbols;
	};

	extern const void *VarDecl;

	#define IS_VAR_DECL(obj) ((obj) && (((struct VarDecl *) obj)->type == VarDecl))

#endif // decl_var_h
