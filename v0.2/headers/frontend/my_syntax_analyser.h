#if !defined(my_syntax_analyser_h)
	#define my_syntax_analyser_h

	#include <stdbool.h>
	#include <stdint.h>

	struct SyntaxAnalyser {
		const void *type;

		const void *lexer;

		uint32_t cur_token_index;

		const struct Token *const *tokens;

		bool had_error;

		bool in_panic_mode;

		uint32_t err_count;	
	};

	extern const void *SyntaxAnalyser;

	#define IS_SYNTAX_ANALYSER(obj) ((obj) && (((struct SyntaxAnalyser *) obj)->type == SyntaxAnalyser))
	#define SA_GET_HAD_ERROR(syntax_analyser) (((struct SyntaxAnalyser *) syntax_analyser)->had_error)

	void perform_analysis(struct SyntaxAnalyser *syntax_analyser);

#endif // my_syntax_analyser_h
