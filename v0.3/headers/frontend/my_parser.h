#if !defined(parser_h)
	#define parser_h

	#include <stdbool.h>
	#include <stdint.h>

	struct Parser {
		const void *type;

		const void *source;

		uint64_t tokens_count;
		const void *const *tokens;

		uint64_t token_index;

		void *symbol_table_stack;
		void *program_ast;

		bool is_import;

		uint32_t err_count;
		bool had_error;

		bool parsing_loop;
		bool parsing_fun;
		bool parsing_class;
		bool parsing_class_has_superclass;
	};

	extern const void *Parser;

	#define IS_PARSER(ptr) ((ptr) && (((const struct Parser *) ptr)->type == Parser))

	#define PARSER_SYMBOL_TABLE_STACK(parser) 	(((const struct Parser *) parser)->symbol_table_stack)
	#define PARSER_PROGRAM_AST(parser) 			(((const struct Parser *) parser)->program_ast)
	#define PARSER_HAD_ERROR(parser)			(((const struct Parser *) parser)->had_error)

	void parse(void *parser, bool is_import, const void *lexer);

#endif // parser_h
