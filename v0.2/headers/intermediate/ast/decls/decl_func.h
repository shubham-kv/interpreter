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

	#define IS_FUN_DECL(ptr) ((ptr) && (FunDecl == ((struct FunDecl *) ptr)->type))


	#define FUN_DECL_GET_SYMBOL(fun_decl_ptr) 			(((struct FunDecl *) fun_decl_ptr)->symbol)

	#define FUN_DECL_GET_SYMBOL_TABLE(fun_decl_ptr) 	(((struct FunDecl *) fun_decl_ptr)->symbol_table)

	#define FUN_DECL_GET_PARAM_COUNT(fun_decl_ptr) 		(((struct FunDecl *) fun_decl_ptr)->param_count)

	#define FUN_DECL_GET_PARAM_SYMBOLS(fun_decl_ptr)	(((struct FunDecl *) fun_decl_ptr)->param_symbols)

	#define FUN_DECL_GET_DECLARATIONS(fun_decl_ptr) 	(((struct FunDecl *) fun_decl_ptr)->declarations)



	void fun_decl_set_declarations(void *fun_decl, void **declarations);

#endif // decl_fun_h
