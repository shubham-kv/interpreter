#if !defined(program_ast_h)
	#define program_ast_h

	#include <stdbool.h>

	struct ProgramAst {
		const void *type;

		// void *program_symbol;

		void *symbol_table;

		void **declarations;
	};

	extern const void *ProgramAst;

	#define IS_PROGRAM_AST(obj) ((obj) && (((struct ProgramAst *) obj)->type == ProgramAst))
	// bool is_program_ast(const void *object);

#endif // program_ast_h
