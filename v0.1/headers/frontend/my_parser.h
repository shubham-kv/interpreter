#if !defined(parser_h)
	#define parser_h

	#include <stdbool.h>
	#include <stdint.h>

	struct Parser {
		const void *type;

		const void *lexer;

		uint32_t cur_token_index;
		const void *const *tokens;

		void *symbol_table_stack;
		void *program_ast;

		uint32_t err_count;

		bool had_error;


		bool parsing_loop;
		bool parsing_fun;
		bool parsing_class;
	};

	extern const void *Parser;

	#define IS_PARSER(obj) ((obj) && (((struct Parser *) obj)->type == Parser))

	#define PARSER_GET_SYMBOL_TABLE_STACK(parser) (((struct Parser *) parser)->symbol_table_stack)
	#define PARSER_GET_PROGRAM_AST(parser) (((struct Parser *) parser)->program_ast)
	#define PARSER_HAD_ERROR(parser) (((struct Parser *) parser)->had_error)

	void parse(void *parser);

#endif // parser_h
