#if !defined(interpreter_h)
	#define interpreter_h

	#include <stdbool.h>

	struct Interpreter {
		const void *type;

		const void *source;

		const void *ast_visitor;
		void *runtime_stack;

		void *saved_program_frame;

		bool break_flag;
		bool continue_flag;
		bool ret_flag;
		void *ret_value;
	};

	extern const void *Interpreter;

	#define IS_INTERPRETER(ptr) ((ptr) && (((const struct Interpreter *) ptr)->type == Interpreter))

	void interpret(void *interpreter, const void *parser);

#endif // interpreter_h
