
// #include "my_syntax_analyser.h"

// #include <assert.h>

// #include <stdlib.h>
// #include <stdarg.h>
// #include <stdbool.h>
// #include <stddef.h>
// #include <stdint.h>
// #include <stdio.h>
// #include <string.h>
// #include <time.h>

// #include "main.h"

// #include "type.h"
// #include "object.h"
// #include "memory.h"

// #include "my_lexer.h"
// #include "token_type.h"
// #include "token.h"
// #include "value.h"
// // #include "parse_err_handler.h"

// #include "ast_object.h"
// #include "ast_node_headers.h"


// #define MAX_ERROR 8


// void *sa_constructor(void *self, va_list *ap);
// void *sa_destructor(void *self);


// const struct Type _type_syntax_analyser = {
// 	sizeof(struct SyntaxAnalyser),
// 	sa_constructor,
// 	sa_destructor
// };

// const void *SyntaxAnalyser = &_type_syntax_analyser;



// // ... = new(SyntaxAnalyser, lexer);

// void *sa_constructor(void *_self, va_list *ap) {
// 	struct SyntaxAnalyser *self = _self;
// 	assert(IS_SYNTAX_ANALYSER(self));

// 	self->lexer = va_arg(*ap, const void *);
// 	assert(IS_LEXER(self->lexer));

// 	self->cur_token_index = 0;
// 	self->tokens = (const struct Token*const *) LEXER_GET_TOKENS(self->lexer);
// 	self->had_error = false;

// 	return _self;
// }


// void *sa_destructor(void *_self) {
// 	return _self;
// }



// // bool sa_had_error(const void *_self) {
// // 	const struct SyntaxAnalyser *self = _self;
// // 	assert(IS_SYNTAX_ANALYSER(self));

// // 	return self->had_error;
// // }


// #define PREV_TOKEN(syntax_analyser) ((struct Token *)syntax_analyser->tokens[(syntax_analyser->cur_token_index) - 1])

// #define CUR_TOKEN(syntax_analyser) ((struct Token *)syntax_analyser->tokens[syntax_analyser->cur_token_index])

// #define SA_MATCH(self, expected) ((self->tokens[self->cur_token_index])->token_type.ordinal == (expected.ordinal))


// bool sa_match_any(const struct SyntaxAnalyser *self, uint32_t count, ...) {
// 	va_list ap;
// 	va_start(ap, count);
// 	const struct Token *token = CUR_TOKEN(self);

// 	for(uint32_t i = 0; i < count; i++) {
// 		const struct TokenType expected = va_arg(ap, const struct TokenType);

// 		if(token->token_type.ordinal == expected.ordinal) {
// 			va_end(ap);
// 			return true;
// 		}
// 	}

// 	va_end(ap);
// 	return false;
// }

// void sa_advance(struct SyntaxAnalyser *self) {
// 	struct Token *token = CUR_TOKEN(self);

// 	if(token->token_type.ordinal != TOKEN_TYPE_EOF.ordinal)
// 		self->cur_token_index++;
// }

// bool sa_consume(struct SyntaxAnalyser *self, struct TokenType expected) {
// 	if(SA_MATCH(self, expected)) {
// 		sa_advance(self);
// 		return true;
// 	}
// 	return false;
// }

// bool sa_consume_any(struct SyntaxAnalyser *self, uint32_t count, ...) {
// 	va_list ap;
// 	va_start(ap, count);
// 	struct Token *cur_token = CUR_TOKEN(self);

// 	for(uint32_t i = 0; i < count; i++) {
// 		const struct TokenType expected = va_arg(ap, const struct TokenType);

// 		if(cur_token->token_type.ordinal == expected.ordinal) {
// 			va_end(ap);
// 			sa_advance(self);
// 			return true;
// 		}
// 	}

// 	va_end(ap);
// 	return false;
// }



// void sa_flag_error_va(
// 	struct SyntaxAnalyser *self,
// 	bool display_line, const uint32_t line_number,
// 	const char *err_line, const uint32_t err_index, const uint32_t err_token_length,
// 	const char *err_msg, va_list *ap
// ) {
// 	fprintf(stderr, "\n");
// 	vfprintf(stderr, err_msg, *ap);
// 	fprintf(stderr, "\n");

// 	if(display_line) {
// 		fprintf(stderr, "-%03d-| ", line_number);

// 		for(uint32_t i = 0; err_line[i] != '\0'; i++) {
// 			char c = err_line[i];
			
// 			if('\t' == c)
// 				c = ' ';

// 			fprintf(stderr, "%c", c);
// 		}

// 		for(uint32_t i = 0; i < (err_index + 7); i++)
// 			fprintf(stderr, " ");

// 		for(uint32_t i = 0; i < err_token_length; i++)
// 			fprintf(stderr, "^");
		
// 		fprintf(stderr, "\n\n");
// 	}

// 	if(is_prompt_mode()) return;

// 	if(self->err_count >= MAX_ERROR) {
// 		fprintf(stderr, "\nToo many errors!\n");
// 		exit(EXIT_FAILURE);
// 	}

// 	self->err_count++;
// }


// void sa_raise_error(
// 	struct SyntaxAnalyser *self,
// 	const struct Token *err_token,
// 	const char *err_msg, ...
// ) {

// 	va_list ap;
// 	va_start(ap, err_msg);

// 	if(NULL == err_token) {
// 		sa_flag_error_va(self, false, 0, NULL, 0, 0, err_msg, &ap);
// 	}
// 	else {
// 		const int line_index = err_token->line_index;
// 		const char *err_line = LEXER_GET_LINE(self->lexer, line_index);
// 		const int err_index = err_token->start_index;

// 		self->had_error = true;
// 		self->in_panic_mode = true;

// 		sa_flag_error_va(
// 			self, true, (line_index + 1),
// 			err_line, err_index, err_token->length, err_msg, &ap
// 		);
// 	}

// 	va_end(ap);
// }




// /*
// |==============================>
// | Declaration analyser methods.
// |==============================>
// */
// bool analyse_program(struct SyntaxAnalyser *self);

// bool analyse_declaration(struct SyntaxAnalyser *self, bool raise_error);

// bool analyse_var_decl(struct SyntaxAnalyser *self, bool raise_error);
// bool analyse_var_init(struct SyntaxAnalyser *self, const struct Token *prev_token, bool raise_error);



// /*
// |==============================>
// | Statement analyser methods.
// |==============================>
// */
// bool analyse_stmt(struct SyntaxAnalyser *self, bool raise_error);

// bool analyse_expr_stmt	(struct SyntaxAnalyser *self, bool raise_error);
// // bool analyse_print_stmt	(struct SyntaxAnalyser *self, bool raise_error);

// bool analyse_if_stmt	(struct SyntaxAnalyser *self, bool raise_error);
// bool analyse_when_stmt	(struct SyntaxAnalyser *self, bool raise_error);

// bool analyse_loop_stmt	(struct SyntaxAnalyser *self, bool raise_error);
// bool analyse_repeat_stmt(struct SyntaxAnalyser *self, bool raise_error);

// bool analyse_block_stmt	(struct SyntaxAnalyser *self, bool raise_error);


// /*
// |==============================>
// | Expression analyser methods.
// |==============================>
// */
// bool analyse_expr(struct SyntaxAnalyser *self, bool raise_error);

// bool analyse_logical_or	(struct SyntaxAnalyser *self, bool raise_error);
// bool analyse_logical_and(struct SyntaxAnalyser *self, bool raise_error);

// bool analyse_equality	(struct SyntaxAnalyser *self, bool raise_error);
// bool analyse_comparison	(struct SyntaxAnalyser *self, bool raise_error);

// bool analyse_term	(struct SyntaxAnalyser *self, bool raise_error);
// bool analyse_factor	(struct SyntaxAnalyser *self, bool raise_error);

// bool analyse_unary	(struct SyntaxAnalyser *self, bool raise_error);
// bool analyse_atom	(struct SyntaxAnalyser *self, bool raise_error);



// void recover(struct SyntaxAnalyser *self) {
// 	while(
// 		!sa_match_any(
// 			self, 6,
// 			TOKEN_TYPE_IF, TOKEN_TYPE_WHEN,
// 			TOKEN_TYPE_LOOP, TOKEN_TYPE_REPEAT,
// 			TOKEN_TYPE_PRINT, TOKEN_TYPE_EOF
// 		)
// 	) {
// 		sa_advance(self);
// 	}
// }


// // program -> declaration* EOF ;

// bool analyse_program(struct SyntaxAnalyser *self) {
// 	for(
// 		const struct Token *token = CUR_TOKEN(self);
// 		token->token_type.ordinal != TOKEN_TYPE_EOF.ordinal;
// 		token = CUR_TOKEN(self)
// 	) {
// 		analyse_declaration(self, true);

// 		// if in panic mode, dump tokens to synchronize
// 		if(self->in_panic_mode) {
// 			recover(self);
// 			self->in_panic_mode = false;
// 		}
// 	}
// }


// bool analyse_declaration(struct SyntaxAnalyser *self, bool raise_error) {
// 	if(SA_MATCH(self, TOKEN_TYPE_VAR)) {
// 		return analyse_var_decl(self, true);
// 	}

// 	return analyse_stmt(self, true);
// }




// // var_decl -> "var" var_init ("," var_init) ";" ;

// bool analyse_var_decl(struct SyntaxAnalyser *self, bool raise_error) {
// 	const struct Token *var_token = CUR_TOKEN(self);
// 	sa_advance(self);

// 	if(!analyse_var_init(self, var_token, true))
// 		return false;

// 	const struct Token *comma_token = NULL;

// 	while(SA_MATCH(self, TOKEN_TYPE_COMMA)) {
// 		comma_token = CUR_TOKEN(self);
// 		sa_advance(self);

// 		analyse_var_init(self, comma_token, true);
// 	}

// 	// synchronize until a ";"

// 	if(!sa_consume(self, TOKEN_TYPE_SEMICOLON)) {
// 		sa_raise_error(
// 			self, var_token,
// 			"SyntaxError: Expected ';' after var statement."
// 		);

// 		return false;
// 	}

// 	return true;
// }


// // var_init -> IDENTIFIER ("=" expr)? ;

// bool analyse_var_init(struct SyntaxAnalyser *self, const struct Token *prev_token, bool raise_error) {
// 	const struct Token *id_token = NULL;

// 	if(SA_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
// 		id_token = CUR_TOKEN(self);
// 		sa_advance(self);
// 	}
// 	else {
// 		const struct Token *cur = CUR_TOKEN(self);

// 		if(SA_MATCH(self, TOKEN_TYPE_EOF)) {
// 			sa_raise_error(
// 				self, prev_token,
// 				"SyntaxError: Expected Identifier after '%.*s'.", prev_token->length, prev_token->start
// 			);
// 		} 
// 		else {
// 			sa_advance(self);
			
// 			sa_raise_error(
// 				self, cur,
// 				"SyntaxError: Unexpected '%.*s' after '%.*s'.", cur->length, cur->start, prev_token->length, prev_token->start
// 			);
// 		}

// 		return false;
// 	}

// 	if(SA_MATCH(self, TOKEN_TYPE_EQUAL)) {
// 		const struct Token *equal_token = CUR_TOKEN(self);
// 		sa_advance(self);

// 		if(!analyse_expr(self, true))
// 			return false;
// 	}

// 	return true;
// }


// // stmt -> expr_stmt | if_stmt | loop_stmt | repeat_stmt | print_stmt | block_stmt

// bool analyse_stmt(struct SyntaxAnalyser *self, bool raise_error) {

// 	if(SA_MATCH(self, TOKEN_TYPE_IF))
// 		return analyse_if_stmt(self, true);

// 	if(SA_MATCH(self, TOKEN_TYPE_WHEN))
// 		return analyse_when_stmt(self, true);

// 	else if(SA_MATCH(self, TOKEN_TYPE_LOOP))
// 		return analyse_loop_stmt(self, true);

// 	else if(SA_MATCH(self, TOKEN_TYPE_REPEAT))
// 		return analyse_repeat_stmt(self, true);

// 	// else if(SA_MATCH(self, TOKEN_TYPE_PRINT))
// 	// 	return analyse_print_stmt(self, true);

// 	else if(SA_MATCH(self, TOKEN_TYPE_LCURLY_BRACE))
// 		return analyse_block_stmt(self, true);

// 	else
// 		return analyse_expr_stmt(self, true);
// }



// // if_stmt -> "if" "(" expr ")" stmt ("else" stmt)? ;

// bool analyse_if_stmt(struct SyntaxAnalyser *self, bool raise_error) {
// 	const struct Token *if_token = CUR_TOKEN(self);
// 	sa_advance(self);

// 	const struct Token *lparen_token = NULL;

// 	if(SA_MATCH(self, TOKEN_TYPE_LPAREN)) {
// 		lparen_token = CUR_TOKEN(self);
// 		sa_advance(self);
// 	}
// 	else {
// 		sa_raise_error(
// 			self, if_token,
// 			"SyntaxError: Expected some expression within parenthesis after if."
// 		);
// 		return false;
// 	}

// 	if(!analyse_expr(self, true))
// 		return false;

// 	if(!sa_consume(self, TOKEN_TYPE_RPAREN)) {
// 		sa_raise_error(
// 			self, lparen_token,
// 			"SyntaxError: Expected corresponding ')' for '('."
// 		);
// 		return false;
// 	}

// 	if(!analyse_declaration(self, true))
// 		return false;

// 	if(SA_MATCH(self, TOKEN_TYPE_ELSE)) {
// 		const struct Token *else_token = CUR_TOKEN(self);
// 		sa_advance(self);

// 		if(!analyse_stmt(self, true))
// 			return false;
// 	}
// 	return true;
// }


// /*
// when_stmt	->  "when" "(" expr ")" "{"
// 					(term ("," term)* "->" stmt)+
// 					("else" -> stmt)?
// 				"}" ;
// */

// bool analyse_when_case(struct SyntaxAnalyser *self, bool raise_error);

// bool analyse_when_stmt(struct SyntaxAnalyser *self, bool raise_error) {
// 	const struct Token *when_token = CUR_TOKEN(self);
// 	sa_advance(self);

// 	const struct Token *lparen_token = NULL;
// 	const struct Token *rparen_token = NULL;
// 	const struct Token *lbrace_token = NULL;

// 	if(SA_MATCH(self, TOKEN_TYPE_LPAREN)) {
// 		lparen_token = CUR_TOKEN(self);
// 		sa_advance(self);
// 	} else {
// 		sa_raise_error(
// 			self, when_token,
// 			"SyntaxError: Expected some expression within parenthesis after when."
// 		);

// 		return false;
// 	}

// 	analyse_expr(self, true);

// 	// if(!analyse_expr(self, true))
// 	// 	return false;

// 	if(SA_MATCH(self, TOKEN_TYPE_RPAREN)) {
// 		rparen_token = CUR_TOKEN(self);
// 		sa_advance(self);
// 	}
// 	else {
// 		sa_raise_error(
// 			self, lparen_token,
// 			"SyntaxError: Expected corresponding ')' for '('."
// 		);

// 		return false;
// 	}

// 	if(SA_MATCH(self, TOKEN_TYPE_LCURLY_BRACE)) {
// 		lbrace_token = CUR_TOKEN(self);
// 		sa_advance(self);
// 	}
// 	else {
// 		sa_raise_error(
// 			self, rparen_token,
// 			"SyntaxError: Expected some case statements enclosed in curly braces after when's ')'."
// 		);

// 		return false;
// 	}

// 	if(!analyse_when_case(self, true))
// 		return false;



// 	while(!sa_match_any(self, 3, TOKEN_TYPE_ELSE, TOKEN_TYPE_RCURLY_BRACE, TOKEN_TYPE_EOF)) {
// 		if(!analyse_when_case(self, true))
// 			return false;
// 	}

// 	if(SA_MATCH(self, TOKEN_TYPE_ELSE)) {
// 		const struct Token *else_token = CUR_TOKEN(self);
// 		sa_advance(self);

// 		const struct Token *arrow_token = NULL;

// 		if(SA_MATCH(self, TOKEN_TYPE_DASH_ARROW)) {
// 			arrow_token = CUR_TOKEN(self);
// 			sa_advance(self);
// 		}
// 		else {
// 			sa_raise_error(
// 				self, else_token,
// 				"SyntaxError: Expected '->' after 'else'."
// 			);

// 			return true;
// 		}

// 		if(!analyse_stmt(self, true))
// 			return false;

// 		// else_stmt = stmt(self);
// 		// if(NULL == else_stmt) {
// 		// 	sa_raise_error(
// 		// 		self, arrow_token,
// 		// 		"SyntaxError: Expected some statement after '->'."
// 		// 	);
// 		// 	return false;
// 		// }
// 	}

// 	if(!sa_consume(self, TOKEN_TYPE_RCURLY_BRACE)) {
// 		sa_raise_error(
// 			self, lbrace_token,
// 			"SyntaxError: Expected corresponding '}' for '{'."
// 		);

// 		return false;
// 	}

// 	return true;
// }


// // when_case -> term ("," term)* "->" stmt

// bool analyse_when_case(struct SyntaxAnalyser *self, bool raise_error) {

// 	if(!analyse_term(self, true))
// 		return false;

// 	const struct Token *comma_token = NULL;
// 	const struct Token *last_token = CUR_TOKEN(self);

// 	while(SA_MATCH(self, TOKEN_TYPE_COMMA)) {
// 		comma_token = CUR_TOKEN(self);
// 		sa_advance(self);

// 		last_token = CUR_TOKEN(self);

// 		if(!analyse_term(self, true))
// 			return false;

// 		// case_expr = term(self);
// 		// if(NULL == case_expr) {
// 		// 	raise_error(
// 		// 		self, false, comma_token,
// 		// 		"SyntaxError: Expected some expression after ','."
// 		// 	);
// 		// 	return NULL;
// 		// }
// 	}

// 	const struct Token *arrow_token = NULL;

// 	if(SA_MATCH(self, TOKEN_TYPE_DASH_ARROW)) {
// 		arrow_token = CUR_TOKEN(self);
// 		sa_advance(self);
// 	}
// 	else {
// 		sa_raise_error(
// 			self, last_token,
// 			"SyntaxError: Expected '->' after expression."
// 		);

// 		return false;
// 	}

// 	if(!analyse_stmt(self, true))
// 		return false;

// 	// void *case_stmt = stmt(self);
// 	// if(NULL == case_stmt) {
// 	// 	raise_error(
// 	// 		self, false, arrow_token,
// 	// 		"SyntaxError: Expected some statement after '->'."
// 	// 	);

// 	// 	return NULL;
// 	// }

// 	return true;
// }


// // loop_stmt -> "loop" "(" term ("->" | "=>") term (";" term)? ) stmt ;
// // loop_stmt -> "loop" "(" expr ")" stmt ;

// bool analyse_loop_stmt(struct SyntaxAnalyser *self, bool raise_error) {
// 	const struct Token *loop_token = CUR_TOKEN(self);
// 	sa_advance(self);

// 	const struct Token *lparen_token = NULL;

// 	if(SA_MATCH(self, TOKEN_TYPE_LPAREN)) {
// 		lparen_token = CUR_TOKEN(self);
// 		sa_advance(self);
// 	}
// 	else {
// 		sa_raise_error(
// 			self, loop_token,
// 			"SyntaxError: Expected '(' after loop."
// 		);
// 		return false;
// 	}

// 	const struct Token *arrow_token = NULL;

// 	if(!analyse_term(self, true))
// 		return false;

// 	// const void *start_expr = expr(self);
// 	// if(NULL == start_expr) {
// 	// 	raise_error(
// 	// 		self, false, lparen_token,
// 	// 		"SyntaxError: Expected some expression after '('."
// 	// 	);
// 	// 	return NULL;
// 	// }

// 	if(SA_MATCH(self, TOKEN_TYPE_RPAREN)) {
// 		sa_advance(self);
// 	}
// 	else {

// 		if(sa_match_any(self, 2, TOKEN_TYPE_DASH_ARROW, TOKEN_TYPE_EQUAL_ARROW)) {
// 			arrow_token = CUR_TOKEN(self);
// 			sa_advance(self);
// 		}
// 		else {
// 			sa_raise_error(
// 				self, loop_token,
// 				"SyntaxError: Expected '->' or '=>' after expression for loop."
// 			);

// 			return false;
// 		}

// 		if(!analyse_term(self, true))
// 			return false;

// 		// end_expr = term(self);

// 		// if(NULL == end_expr) {
// 		// 	raise_error(
// 		// 		self, false, arrow_token,
// 		// 		"SyntaxError: Expected some expression after arrow."
// 		// 	);

// 		// 	return NULL;
// 		// }

// 		if(SA_MATCH(self, TOKEN_TYPE_SEMICOLON)) {
// 			const struct Token *semicolon = CUR_TOKEN(self);
// 			sa_advance(self);

// 			if(!analyse_term(self, true))
// 				return false;

// 			// stesa_expr = term(self);			
// 			// if(NULL == stesa_expr) {
// 			// 	sa_raise_error(
// 			// 		self, semicolon,
// 			// 		"SyntaxError: Expected some step value after ';'."
// 			// 	);

// 			// 	return NULL;
// 			// }
// 		}

// 		if(SA_MATCH(self, TOKEN_TYPE_RPAREN)) {
// 			sa_advance(self);
// 		}
// 		else {
// 			sa_raise_error(
// 				self, lparen_token,
// 				"SyntaxError: Expected corresponding ')' for '('."
// 			);

// 			return false;
// 		}
// 	}



// 	if(SA_MATCH(self, TOKEN_TYPE_LCURLY_BRACE)) {
// 		const struct Token *lbrace_token = CUR_TOKEN(self);
// 		sa_advance(self);

// 		while(!sa_match_any(self, 2, TOKEN_TYPE_EOF, TOKEN_TYPE_RCURLY_BRACE)) {

// 			if(!analyse_declaration(self, true))
// 				return false;

// 			// void *child_stmt = declaration(self);
// 			// if(NULL == child_stmt) {
// 			// 	if(child_stmts_count == 0) {
// 			// 		raise_error(
// 			// 			self, false, lbrace_token,
// 			// 			"SyntaxError: Expected some statements after '{'."
// 			// 		);
// 			// 	}

// 			// 	return NULL;
// 			// }
// 		}

// 		if(!sa_consume(self, TOKEN_TYPE_RCURLY_BRACE)) {
// 			sa_raise_error(
// 				self, lbrace_token,
// 				"SyntaxError: Expected corresponding '}' for '{'."
// 			);

// 			return false;
// 		}
// 	}
// 	else {
// 		if(!analyse_declaration(self, true))
// 				return false;

// 		// void *child_stmt = declaration(self);
// 		// if(NULL == child_stmt) {
// 		// 	symtabstack_pop(self->symbol_table_stack);	// repeat_symbol_table
// 		// 	raise_error(
// 		// 		self, false, loop_token,
// 		// 		"SyntaxError: Expected some child statement for loop."
// 		// 	);

// 		// 	return NULL;
// 		// }
// 	}

// 	return true;
// }


// // repeat_stmt -> "repeat" "(" term ")" stmt ;

// bool analyse_repeat_stmt(struct SyntaxAnalyser *self, bool raise_error) {
// 	const struct Token *repeat_token = CUR_TOKEN(self);
// 	sa_advance(self);

// 	const struct Token *lparen_token = NULL;

// 	if(SA_MATCH(self, TOKEN_TYPE_LPAREN)) {
// 		lparen_token = CUR_TOKEN(self);
// 		sa_advance(self);
// 	}
// 	else {
// 		sa_raise_error(
// 			self, repeat_token,
// 			"SyntaxError: Expected '(' after repeat."
// 		);		
// 		return false;
// 	}


// 	if(!analyse_term(self, true))
// 		return false;

// 	// void *expression = term(self);
// 	// if(NULL == expression) {
// 	// 	raise_error(
// 	// 		self, false, lparen_token,
// 	// 		"SyntaxError: Expected some expression after '('."
// 	// 	);
// 	// 	return NULL;
// 	// }

// 	const struct Token *cur = CUR_TOKEN(self);

// 	if((!SA_MATCH(self, TOKEN_TYPE_EOF)) &&
// 		sa_match_any(self, 8, TOKEN_TYPE_LOGICAL_AND, TOKEN_TYPE_LOGICAL_OR, TOKEN_TYPE_EQUAL_EQUAL, TOKEN_TYPE_NOT_EQUAL,
// 							TOKEN_TYPE_LESS_THAN, TOKEN_TYPE_LESS_THAN_EQUALS, TOKEN_TYPE_GREATER_THAN, TOKEN_TYPE_GREATER_THAN_EQUALS)
// 	) {
// 		sa_raise_error(
// 			self, cur,
// 			"Error: Unexpected '%.*s' in repeat's expression.", cur->length, cur->start
// 		);
// 		return false;
// 	}

// 	if(!sa_consume(self, TOKEN_TYPE_RPAREN)) {
// 		sa_raise_error(
// 			self, lparen_token,
// 			"SyntaxError: Expected corresponding ')' for '('."
// 		);

// 		return false;
// 	}


// 	if(SA_MATCH(self, TOKEN_TYPE_LCURLY_BRACE)) {
// 		const struct Token *lbrace_token = CUR_TOKEN(self);
// 		sa_advance(self);

// 		while(!sa_match_any(self, 2, TOKEN_TYPE_EOF, TOKEN_TYPE_RCURLY_BRACE)) {

// 			if(!analyse_declaration(self, true))
// 				return false;

// 			// void *child_stmt = declaration(self);
// 			// if(NULL == child_stmt) {
// 			// 	if(child_stmts_count == 0) {
// 			// 		raise_error(
// 			// 			self, false, lbrace_token,
// 			// 			"SyntaxError: Expected some statements after '{'."
// 			// 		);
// 			// 	}
// 			// 	return NULL;
// 			// }
// 		}

// 		if(!sa_consume(self, TOKEN_TYPE_RCURLY_BRACE)) {
// 			sa_raise_error(
// 				self, lbrace_token,
// 				"SyntaxError: Expected corresponding '}' for '{'."
// 			);

// 			return false;
// 		}
// 	}
// 	else {
// 		if(!analyse_declaration(self, true))
// 			return false;

// 		// if(NULL == child_stmt) {
// 		// 	raise_error(
// 		// 		self, false, repeat_token,
// 		// 		"SyntaxError: Expected some child statement for repeat."
// 		// 	);

// 		// 	return NULL;
// 		// }
// 	}

// 	return true;
// }


// // print_stmt -> "print" expr ;

// // bool analyse_print_stmt(struct SyntaxAnalyser *self, bool raise_error) {
// // 	const struct Token *print_token = CUR_TOKEN(self);
// // 	sa_advance(self);

// // 	if(!analyse_expr(self, true))
// // 		return false;

// // 	if(!sa_consume(self, TOKEN_TYPE_SEMICOLON)) {
// // 		sa_raise_error(
// // 			self, print_token,
// // 			"SyntaxError: Expected ';' after print statement."
// // 		);

// // 		return false;
// // 	}

// // 	return true;
// // }



// // block_stmt -> "{" declaration* "}" ;

// bool analyse_block_stmt(struct SyntaxAnalyser *self, bool raise_error) {

// 	const struct Token *lbracke_token = CUR_TOKEN(self);
// 	sa_advance(self);

// 	while(!sa_match_any(self, 2, TOKEN_TYPE_RCURLY_BRACE, TOKEN_TYPE_EOF)) {

// 		if(!analyse_declaration(self, true))
// 			return false;
// 	}

// 	if(!sa_consume(self, TOKEN_TYPE_RCURLY_BRACE)) {
// 		sa_raise_error(
// 			self, lbracke_token,
// 			"SyntaxError: Expected corresponding '}' for '{'."
// 		);
// 		return false;
// 	}

// 	return true;
// }



// // expr_stmt -> expr ";" ;
// /*
// could have
// expr_stmt 	->  IDENTIFIER = expr ";"			|
// 				("++" | "--") IDENTIFIER ";"	|
// 				IDENTIFIER "(" params ")" ";"	|

// */

// bool analyse_expr_stmt(struct SyntaxAnalyser *self, bool raise_error) {
// 	// printf("analyse_expr_stmt()\n");

// 	const struct Token *token = CUR_TOKEN(self);
// 	uint32_t i = self->cur_token_index;

// 	if(!analyse_expr(self, true))
// 		return false;

// 	if(!sa_consume(self, TOKEN_TYPE_SEMICOLON)) {
// 		sa_raise_error(
// 			self, token,
// 			"SyntaxError: Expected ';' after this expression."
// 		);
// 		return false;
// 	}

// 	return true;
// }





// // expr -> IDENTIFIER "=" expr | logical_or ;

// bool analyse_expr(struct SyntaxAnalyser *self, bool raise_error) {
// 	// printf("analyse_expr()\n");

// 	if(!analyse_logical_or(self, raise_error))
// 		return false;
	
// 	if(SA_MATCH(self, TOKEN_TYPE_EQUAL)) {
// 		const struct Token *equal_token = CUR_TOKEN(self);
// 		const struct Token *prev_token = PREV_TOKEN(self);
// 		sa_advance(self);

// 		if(prev_token->token_type.ordinal != TOKEN_TYPE_IDENTIFIER.ordinal) {
// 			if(raise_error)
// 				sa_raise_error(
// 					self, prev_token,
// 					"Error: Invalid lvalue before '='."
// 				);
// 			return false;
// 		}

// 		if(!analyse_expr(self, true)) {
// 			if(raise_error)
// 				sa_raise_error(
// 					self, equal_token,
// 					"SyntaxError: Expected expression after '='."
// 				);
// 			return false;
// 		}
// 	}

// 	return true;
// }


// bool analyse_binary(
// 	struct SyntaxAnalyser *self, bool raise_error,
// 	bool (*analyser_fun)(struct SyntaxAnalyser *self, bool raise_error),
// 	uint32_t count, ...
// ) {

// 	if(!analyser_fun(self, raise_error))
// 		return false;

// 	va_list ap;
// 	va_start(ap, count);

// 	for(uint32_t i = 0; i < count; i++) {
// 		const struct TokenType expected = va_arg(ap, const struct TokenType);

// 		while(SA_MATCH(self, expected)) {
// 			sa_advance(self);

// 			if(!analyser_fun(self, raise_error)) {
// 				va_end(ap);
// 				return false;
// 			}
// 		}
// 	}

// 	va_end(ap);
// 	return true;
// }







// // logical_or -> logical_and ( "||" logical_and )*

// bool analyse_logical_or(struct SyntaxAnalyser *self, bool raise_error) {

// 	return analyse_binary(
// 		self, raise_error,
// 		analyse_logical_and, 1, TOKEN_TYPE_LOGICAL_OR
// 	);
// }



// // logical_and -> equality ("&&" equality )*

// bool analyse_logical_and(struct SyntaxAnalyser *self, bool raise_error) {

// 	return analyse_binary(
// 		self, raise_error,
// 		analyse_equality, 1, TOKEN_TYPE_LOGICAL_AND
// 	);
// }



// // equality -> comparison ( "==" | "!=" comparison )*

// bool analyse_equality(struct SyntaxAnalyser *self, bool raise_error) {

// 	return analyse_binary(
// 		self, raise_error, analyse_comparison, 2,
// 		TOKEN_TYPE_EQUAL_EQUAL, TOKEN_TYPE_NOT_EQUAL
// 	);
// }



// // comparison -> term  ( ( "<" | "<=" | ">" | ">=" ) term )*

// bool analyse_comparison(struct SyntaxAnalyser *self, bool raise_error) {

// 	return analyse_binary(
// 		self, raise_error, analyse_term, 4, 
// 		TOKEN_TYPE_LESS_THAN, TOKEN_TYPE_LESS_THAN_EQUALS,
// 		TOKEN_TYPE_GREATER_THAN, TOKEN_TYPE_GREATER_THAN_EQUALS
// 	);
// }



// // term -> factor (('+' | '-') factor)*

// bool analyse_term(struct SyntaxAnalyser *self, bool raise_error) {

// 	return analyse_binary(
// 		self, raise_error, analyse_factor, 2,
// 		TOKEN_TYPE_PLUS, TOKEN_TYPE_MINUS
// 	);
// }



// // factor -> unary (('*' | '/' | '%') unary)*

// bool analyse_factor(struct SyntaxAnalyser *self, bool raise_error) {

// 	return analyse_binary(
// 		self, raise_error, analyse_unary, 3,
// 		TOKEN_TYPE_STAR, TOKEN_TYPE_FORWARD_SLASH, TOKEN_TYPE_PERCENT
// 	);
// }



// // unary -> ('!' | '-' | '+')? unary | primary ;

// bool analyse_unary(struct SyntaxAnalyser *self, bool raise_error) {

// 	if(sa_match_any(self, 3, TOKEN_TYPE_NOT, TOKEN_TYPE_MINUS, TOKEN_TYPE_PLUS)) {
// 		const struct Token *op_token = CUR_TOKEN(self);
// 		sa_advance(self);

// 		if(analyse_unary(self, false))
// 			return true;

// 		if(raise_error && !self->had_error)
// 			sa_raise_error(
// 				self, op_token,
// 				"analyse_unary(): Expected some expression after '%.*s'.", op_token->length, op_token->start
// 			);

// 		return false;
// 	}

// 	return analyse_atom(self, raise_error);
// }


// /*
// atom -> INTEGER | DOUBLE 	|
// 		STRING | IDENTIFIER |
// 		"true" | "false" 	|
// 		"(" expr ")" 		;

// 2. var n = 78 * (var num = 10);

// SyntaxError: Unexpected 'var' at line 2.
// -02-| var n = 78 * (var num = 10);
//                     ^^^

// 4. var n = 78 * (78;

// SyntaxError: Expected corresponding ')' for '('.
// -04-| var n = 78 * (78;
// 				   ^


// 8. var n = ();

// SyntaxError: Unexpected ')' at line 8.
// -08-| var n = ();
// 			   ^

// */

// bool analyse_atom(struct SyntaxAnalyser *self, bool raise_error) {
// 	// printf("analyse_atom()\n");

// 	if(
// 		sa_consume_any(
// 			self, 6, TOKEN_TYPE_INTEGER, TOKEN_TYPE_DOUBLE,
// 			TOKEN_TYPE_STRING, TOKEN_TYPE_IDENTIFIER, TOKEN_TYPE_TRUE, TOKEN_TYPE_FALSE
// 		)
// 	) {
// 		return true;
// 	}

// 	else if(SA_MATCH(self, TOKEN_TYPE_LPAREN)) {
// 		const struct Token *lparen_token = CUR_TOKEN(self);
// 		sa_advance(self);

// 		if(analyse_expr(self, true)) {
// 			if(!sa_consume(self, TOKEN_TYPE_RPAREN)) {
// 				sa_raise_error(
// 					self, lparen_token,
// 					"SyntaxError: Expected corresponding ')' for '('."
// 				);
// 				return false;
// 			}

// 			return true;
// 		}

// 		return false;
// 	}

// 	if(raise_error) {
// 		const struct Token *prev = PREV_TOKEN(self);
// 		const struct Token *cur = CUR_TOKEN(self);
// 		sa_advance(self);

// 		// if(cur->token_type.ordinal == TOKEN_TYPE_EOF.ordinal) {
// 			sa_raise_error(
// 				self, prev,
// 				"SyntaxError: Expected expression after '%.*s'.", prev->length, prev->start
// 			);
// 			return false;
// 		// }

// 		// sa_raise_error(
// 		// 	self, cur,
// 		// 	"SyntaxError: Unexpected '%.*s' at line %d.", cur->length, cur->start, (cur->line_index + 1)
// 		// );
// 	}

// 	return false;
// }







// void perform_analysis(struct SyntaxAnalyser *_self) {
// 	struct SyntaxAnalyser *self = _self;
// 	assert(IS_SYNTAX_ANALYSER(self));

// 	analyse_program(self);
// }
// // /*
