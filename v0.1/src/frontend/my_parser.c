#include "my_parser.h"

#include <assert.h>

#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "main.h"

#include "type.h"
#include "object.h"
#include "memory.h"

#include "my_lexer.h"
#include "token_type.h"
#include "token.h"
#include "value.h"
#include "parse_err_handler.h"

#include "ast_object.h"
#include "ast_node_headers.h"
#include "value_fun.h"
#include "predefined.h"

#include "def_type.h"
#include "symbol.h"
#include "symbol_table.h"
#include "symbol_table_stack.h"


#define MAX_PARSE_ERRORS 16





void *parser_constructor(void *self, va_list *ap);
void *parser_destructor(void *self);

const struct Type _class_parser = {
	sizeof(struct Parser),
	parser_constructor,
	parser_destructor
};


const void *Parser = &_class_parser;



// ... = new(Parser, lexer);

void *parser_constructor(void *_self, va_list *ap) {
	struct Parser *self = _self;
	assert(IS_PARSER(self));

	self->lexer = va_arg(*ap, const void *);
	assert(IS_LEXER(self->lexer));

	self->cur_token_index = 0;
	self->tokens = LEXER_GET_TOKENS(self->lexer);

	self->symbol_table_stack = NULL;
	self->program_ast = NULL;

	self->err_count = 0;
	self->had_error = false;

	return _self;
}


void *parser_destructor(void *_self) {
	struct Parser *self = _self;
	assert(IS_PARSER(self));

	
	if(0 != SYMBOL_TABLE_STACK_GET_SIZE(self->symbol_table_stack)) {
		void *popped_table = (void *) symtabstack_pop(self->symbol_table_stack);
		delete(popped_table);
	}

	if(NULL != self->program_ast)
		delete(self->program_ast);

	if(NULL != self->symbol_table_stack)
		delete(self->symbol_table_stack);

	return _self;
}







#define CUR_TOKEN(parser) ((struct Token *)parser->tokens[parser->cur_token_index])
#define PREV_TOKEN(parser) ((struct Token *)parser->tokens[parser->cur_token_index - 1])

#define P_MATCH(parser, expected) (((struct Token *)parser->tokens[parser->cur_token_index])->token_type.ordinal == (expected.ordinal))
#define P_PEEK(parser, expected) (((struct Token *)parser->tokens[parser->cur_token_index + 1])->token_type.ordinal == (expected.ordinal))




bool p_match_any(const struct Parser *self, int count, ...) {
	va_list ap;
	va_start(ap, count);

	struct Token *token = CUR_TOKEN(self);

	for(uint32_t i = 0; i < count; i++) {
		const struct TokenType expected = va_arg(ap, const struct TokenType);

		if(token->token_type.ordinal == expected.ordinal) {
			va_end(ap);
			return true;
		}
	}

	va_end(ap);
	return false;
}

void p_advance(struct Parser *self) {
	struct Token *token = CUR_TOKEN(self);

	if(token->token_type.ordinal != TOKEN_TYPE_EOF.ordinal)
		self->cur_token_index++;
}

bool p_consume(struct Parser *self, struct TokenType expected) {
	if(P_MATCH(self, expected)) {
		p_advance(self);
		return true;
	}
	return false;
}




void p_flag_error_va(
	struct Parser *self,
	bool display_line, const uint32_t line_number,
	const char *err_line, const uint32_t err_index, const uint32_t err_token_length,
	const char *err_msg, va_list *ap
) {
	fprintf(stderr, "\n");
	vfprintf(stderr, err_msg, *ap);
	fprintf(stderr, "\n");

	if(display_line) {
		fprintf(stderr, "-%03d-| ", line_number);

		for(uint32_t i = 0; err_line[i] != '\0'; i++) {
			char c = err_line[i];
			
			if('\t' == c)
				c = ' ';

			fprintf(stderr, "%c", c);
		}

		for(uint32_t i = 0; i < (err_index + 7); i++)
			fprintf(stderr, " ");

		for(uint32_t i = 0; i < err_token_length; i++)
			fprintf(stderr, "^");
		
		fprintf(stderr, "\n\n");
	}

	if(is_prompt_mode()) return;

	if(self->err_count >= MAX_PARSE_ERRORS) {
		fprintf(stderr, "\nToo many errors!\n");
		exit(EXIT_FAILURE);
	}

	self->err_count++;
}


void p_raise_error(
	struct Parser *self,
	const struct Token *err_token,
	const char *err_msg, ...
) {

	va_list ap;
	va_start(ap, err_msg);

	if(NULL == err_token) {
		p_flag_error_va(self, false, 0, NULL, 0, 0, err_msg, &ap);
	}
	else {
		const int line_index = err_token->line_index;
		const char *err_line = LEXER_GET_LINE(self->lexer, line_index);
		const int err_index = err_token->start_index;

		self->had_error = true;
		
		p_flag_error_va(
			self, true, (line_index + 1),
			err_line, err_index, err_token->length, err_msg, &ap
		);
	}

	va_end(ap);
}




void p_raise_error_expr(
	struct Parser *self,
	const void *expr,
	const char *err_msg, ...
) {

	va_list ap;
	va_start(ap, err_msg);

	const struct Token *lmost = get_leftmost_token(expr);
	const struct Token *rmost = get_rightmost_token(expr);

	fprintf(stderr, "\n");
	vfprintf(stderr, err_msg, ap);
	fprintf(stderr, "\n");

	if(lmost->line_index == rmost->line_index) {
		const int line_number = lmost->line_index + 1;
		fprintf(stderr, "-%03d-| ", line_number);

		const char *err_line = LEXER_GET_LINE(self->lexer, lmost->line_index);
		const int start = lmost->start_index;
		const int end = rmost->start_index + rmost->length;

		for(int i = 0; err_line[i] != '\0'; i++) {
			char c = err_line[i];
			
			if('\t' == c) c = ' ';

			fprintf(stderr, "%c", c);
		}

		for(int i = 0; i < (start + 7); i++)
			fprintf(stderr, " ");
		
		for(int i = 0; i < (end - start); i++)
			fprintf(stderr, "^");

		fprintf(stderr, "\n\n");
	}

	// what if they are on diff. lines
	else {
		
	}

	va_end(ap);

	if(is_prompt_mode())
		return;

	if(self->err_count >= MAX_PARSE_ERRORS) {
		fprintf(stderr, "\nToo many errors!\n");
		exit(EXIT_FAILURE);
	}

	self->err_count++;
}




/*
|==============================>
| Declaration parser methods.
|==============================>
*/
void *program(struct Parser *self);

void *declaration(struct Parser *self);

// void *var_decl(struct Parser *self);
void *var_decl(struct Parser *self, enum DefinitionType def_type);
// void *var_init(struct Parser *self, const struct Token *prev_token);
void *var_init(struct Parser *self, const struct Token *prev_token, enum DefinitionType def_type);


// void *val_decl(struct Parser *self);
void *val_decl(struct Parser *self, enum DefinitionType def_type);
// void *val_init(struct Parser *self, const struct Token *prev_token);
void *val_init(struct Parser *self, const struct Token *prev_token, enum DefinitionType def_type);

// void *fun_decl(struct Parser *self);
void *fun_decl(struct Parser *self, enum DefinitionType def_type);

void *class_decl(struct Parser *self);

/*
|==============================>
| Statement parser methods.
|==============================>
*/
void *stmt(struct Parser *self);

void *expr_stmt(struct Parser *self);
// void *print_stmt(struct Parser *self);

void *if_stmt(struct Parser *self);
void *when_stmt(struct Parser *self);

void *loop_stmt(struct Parser *self);
void *repeat_stmt(struct Parser *self);

void *block_stmt(struct Parser *self);

void *break_stmt(struct Parser *self);
void *continue_stmt(struct Parser *self);
void *ret_stmt(struct Parser *self);


/*
|==============================>
| Expression parser methods.
|==============================>
*/
void *expr(struct Parser *self);

void *assignment(struct Parser *self);

void *logical_or(struct Parser *self);
void *logical_and(struct Parser *self);

void *equality(struct Parser *self);
void *comparison(struct Parser *self);

void *term(struct Parser *self);
void *factor(struct Parser *self);

void *unary(struct Parser *self);
void *molecule(struct Parser *self);
void *atom(struct Parser *self);






// program -> declaration* EOF ;

void *program(struct Parser *self) {
	bool last_decl_null = false;

	void *program_symbol_table = (void *) symtabstack_push(self->symbol_table_stack);

	uint32_t decl_count = 0;
	void **declarations = NULL;

	void *decl = NULL;

	for(
		const struct Token *token = CUR_TOKEN(self);
		token->token_type.ordinal != TOKEN_TYPE_EOF.ordinal;
		token = CUR_TOKEN(self)
	) {
		
		decl = declaration(self);

		declarations = reallocate(declarations, sizeof(void *), ++decl_count);
		declarations[decl_count - 1] = decl;

		// if call to declaration returns null
		// and if cur token is not eof => handle parse error

		if(NULL == decl) {
			last_decl_null = true;

			// const struct Token *cur_token = CUR_TOKEN(self);
			// if(!P_MATCH(self, TOKEN_TYPE_EOF)) {
			// 	p_raise_error(
			// 		self, cur_token,
			// 		"ParseError: Invalid syntax."
			// 	);
			// }

			break;
		}
	}

	if(!last_decl_null) {
		declarations = reallocate(declarations, sizeof(void *), ++decl_count);
		declarations[decl_count - 1] = NULL;
	}

	symtabstack_pop(self->symbol_table_stack);	// program_symbol_table

	return new(ProgramAst, program_symbol_table, declarations);
}


void *declaration(struct Parser *self) {
	if(P_MATCH(self, TOKEN_TYPE_VAR)) {
		return var_decl(self, DEF_TYPE_VARIABLE);
	}
	if(P_MATCH(self, TOKEN_TYPE_VAL)) {
		return val_decl(self, DEF_TYPE_CONSTANT);
	}
	if(P_MATCH(self, TOKEN_TYPE_FUN)) {
		return fun_decl(self, DEF_TYPE_FUN);
	}
	if(P_MATCH(self, TOKEN_TYPE_CLASS)) {
		return class_decl(self);
	}

	return stmt(self);
}




// var_decl -> "var" var_init ("," var_init)* ";" ;
// var_decl -> "var" (IDENTIFIER ("=" expr)?) ("," (IDENTIFIER ("=" expr)?) )* ";" ;

void *var_decl(struct Parser *self, enum DefinitionType def_type) {
	const struct Token *var_token = CUR_TOKEN(self);
	p_advance(self);

	void *symbol = var_init(self, var_token, def_type);

	if(NULL == symbol)
		return NULL;


	uint32_t symbols_count = 0;

	void **symbols = allocate(sizeof(void *), ++symbols_count);
	symbols[symbols_count - 1] = symbol;

	const struct Token *comma_token = NULL;

	while(P_MATCH(self, TOKEN_TYPE_COMMA)) {
		comma_token = CUR_TOKEN(self);
		p_advance(self);

		symbol = var_init(self, comma_token, def_type);

		if(NULL == symbol) {
			free(symbols);
			return NULL;
		}

		symbols = reallocate(symbols, sizeof(void *), ++symbols_count);
		symbols[symbols_count - 1] = symbol;
	}

	if(!p_consume(self, TOKEN_TYPE_SEMICOLON)) {
		free(symbols);

		p_raise_error(
			self, var_token,
			"SyntaxError: Expected ';' after var statement."
		);

		return NULL;
	}

	symbols = reallocate(symbols, sizeof(void *), ++symbols_count);
	symbols[symbols_count - 1] = NULL;

	return new(VarDecl, symbols);
}



// var_init -> IDENTIFIER ("=" expr)? ;

void *var_init(
	struct Parser *self,
	const struct Token *prev_token,
	enum DefinitionType def_type
) {

	const struct Token *id_token = NULL;

	if(P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
		id_token = CUR_TOKEN(self);
		p_advance(self);
	}
	else {
		p_raise_error(
			self, prev_token,
			"SyntaxError: Expected Identifier after '%.*s'.", prev_token->length, prev_token->start
		);

		return NULL;
	}

	char *id = allocate(sizeof(char), id_token->length + 1);
	strncpy(id, id_token->start, id_token->length);

	const void *local_symbol_table = symtabstack_get_local_table(self->symbol_table_stack);

	if(NULL != symbol_table_lookup(local_symbol_table, id)) {

		p_raise_error(
			self, id_token,
			"Error: Redefinition of '%s'.", id
		);

		free(id);
		return NULL;
	}

	
	void *init_expr = NULL;

	if(P_MATCH(self, TOKEN_TYPE_EQUAL)) {
		const struct Token *equal_token = CUR_TOKEN(self);
		p_advance(self);

		init_expr = expr(self);
		
		if(NULL == init_expr) {
			free(id);
			
			p_raise_error(
				self, equal_token,
				"SyntaxError: Expected some expression after '='."
			);
			return NULL;
		}
	}

	// void *symbol = new(Symbol, id, local_symbol_table, DEF_TYPE_VARIABLE);
	void *symbol = new(Symbol, id, local_symbol_table, def_type);

	if(NULL != init_expr)
		symbol_set_attr(symbol, SYM_ATTR_KEY_INIT_EXPR, init_expr);

	symbol_table_define(local_symbol_table, id, symbol);

	return symbol;
}




// val_decl -> "val" val_init ("," val_init)* ";" ;
// val_decl -> "val" (IDENTIFIER "=" expr) ("," (IDENTIFIER "=" expr) )* ";" ;

void *val_decl(struct Parser *self, enum DefinitionType def_type) {
	const struct Token *val_token = CUR_TOKEN(self);
	p_advance(self);

	void *symbol = val_init(self, val_token, def_type);

	if(NULL == symbol)
		return NULL;

	uint32_t symbols_count = 0;

	void **symbols = allocate(sizeof(void *), ++symbols_count);
	symbols[symbols_count - 1] = symbol;

	const struct Token *comma_token = NULL;

	while(P_MATCH(self, TOKEN_TYPE_COMMA)) {
		comma_token = CUR_TOKEN(self);
		p_advance(self);

		symbol = val_init(self, comma_token, def_type);

		if(NULL == symbol) {
			free(symbols);
			return NULL;
		}

		symbols = reallocate(symbols, sizeof(void *), ++symbols_count);
		symbols[symbols_count - 1] = symbol;
	}

	if(!p_consume(self, TOKEN_TYPE_SEMICOLON)) {
		free(symbols);

		p_raise_error(
			self, val_token,
			"SyntaxError: Expected ';' after val statement."
		);

		return NULL;
	}

	symbols = reallocate(symbols, sizeof(void *), ++symbols_count);
	symbols[symbols_count - 1] = NULL;
	
	return new(VarDecl, symbols);
}


// val_init -> IDENTIFIER "=" expr ;

void *val_init(struct Parser *self, const struct Token *prev_token, enum DefinitionType def_type) {
	const struct Token *id_token = NULL;

	if(P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
		id_token = CUR_TOKEN(self);
		p_advance(self);
	}
	else {
		p_raise_error(
			self, prev_token,
			"SyntaxError: Expected Identifier after '%.*s'.", prev_token->length, prev_token->start
		);

		return NULL;
	}


	char *id = allocate(sizeof(char), id_token->length + 1);
	strncpy(id, id_token->start, id_token->length);

	const void *local_symbol_table = symtabstack_get_local_table(self->symbol_table_stack);

	// if(NULL != symtabstack_lookup(self->symbol_table_stack, id)) {
	if(NULL != symbol_table_lookup(local_symbol_table, id)) {
		p_raise_error(
			self, id_token,
			"Error: Redefinition of '%s'.", id
		);
		free(id);
		return NULL;
	}


	if(!P_MATCH(self, TOKEN_TYPE_EQUAL)) {
		free(id);

		p_raise_error(
			self, id_token,
			"Error: Expected expression to initialize constant value '%.*s'.", id_token->length, id_token->start
		);
		return NULL;
	}

	const struct Token *equal_token = CUR_TOKEN(self);
	p_advance(self);

	void *init_expr = expr(self);
	
	if(NULL == init_expr) {
		free(id);
		return NULL;
	}

	// void *symbol = new(Symbol, id, local_symbol_table, DEF_TYPE_CONSTANT);
	void *symbol = new(Symbol, id, local_symbol_table, def_type);
	symbol_set_attr(symbol, SYM_ATTR_KEY_INIT_EXPR, init_expr);

	symbol_table_define(local_symbol_table, id, symbol);

	return symbol;
}



// fun_decl		->  "fun" IDENTIFIER "(" parameters? ")" "{" declaration* "}" ;
// parameters	->  IDENTIFIER ("," IDENTIFIER) ;

void *fun_decl(struct Parser *self, enum DefinitionType def_type) {

	const struct Token *fun_token = CUR_TOKEN(self);
	p_advance(self);

	const struct Token *fun_id_token = NULL;

	if(P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
		fun_id_token = CUR_TOKEN(self);
		p_advance(self);
	}
	else {
		p_raise_error(
			self, fun_token,
			"SyntaxError: Expected funtion's name after '%.*s'", fun_token->length, fun_token->start
		);

		return NULL;
	}

	char *fun_id = allocate(sizeof(char), fun_id_token->length + 1);
	strncpy(fun_id, fun_id_token->start, fun_id_token->length);


	// check if somtheing is already defined with that name
	const void *id_symbol = symtabstack_lookup(self->symbol_table_stack, fun_id);

	if(NULL != id_symbol) {
		if(DEF_TYPE_VARIABLE == SYMBOL_GET_DEF_TYPE(id_symbol)) {
			p_raise_error(
				self, fun_id_token,
				"Error: '%s' already declared as a varible.", fun_id
			);
		}
		else if(DEF_TYPE_CONSTANT == SYMBOL_GET_DEF_TYPE(id_symbol)) {
			p_raise_error(
				self, fun_id_token,
				"Error: '%s' already declared as a constant.", fun_id
			);
		}
		else if(DEF_TYPE_FUN == SYMBOL_GET_DEF_TYPE(id_symbol)) {
			p_raise_error(
				self, fun_id_token,
				"Error: Redefinition of function '%s'.", fun_id
			);
		}
		else {
			p_raise_error(
				self, fun_id_token,
				"Error: Redefinition of '%s'.", fun_id
			);
		}
		free(fun_id);

		return NULL;
	}




	const struct Token *lparen_token = NULL;

	if(P_MATCH(self, TOKEN_TYPE_LPAREN)) {
		lparen_token = CUR_TOKEN(self);
		p_advance(self);
	}
	else {
		p_raise_error(
			self, fun_id_token,
			"SyntaxError: Expected '(' after '%.*s'", fun_id_token->length, fun_id_token->start
		);

		return NULL;
	}

	const void *parent_symbol_table = symtabstack_get_local_table(self->symbol_table_stack);
	const void *fun_symbol_table = symtabstack_push(self->symbol_table_stack);

	uint32_t param_count = 0;
	void **param_symbols = NULL;


	// got some parameters
	if(P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
		const struct Token *param_token = CUR_TOKEN(self);
		p_advance(self);



		char *param_id = allocate(sizeof(char), param_token->length + 1);
		strncpy(param_id, param_token->start, param_token->length);

		void *param_symbol = new(Symbol, param_id, fun_symbol_table, DEF_TYPE_VARIABLE);
		symbol_table_define(fun_symbol_table, param_id, param_symbol);



		param_symbols = allocate(sizeof(void *), ++param_count);
		param_symbols[param_count - 1] = param_symbol;

		while(P_MATCH(self, TOKEN_TYPE_COMMA)) {
			const struct Token *comma_token = CUR_TOKEN(self);
			p_advance(self);

			if(!P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
				delete((void *) symtabstack_pop(self->symbol_table_stack));

				p_raise_error(
					self, comma_token,
					"SyntaxError: Expected a parameter after ','."
				);

				return NULL;
			}

			param_token = CUR_TOKEN(self);
			p_advance(self);

			param_id = allocate(sizeof(char), param_token->length + 1);
			strncpy(param_id, param_token->start, param_token->length);

			if(NULL != symbol_table_lookup(fun_symbol_table, param_id)) {
				delete((void *) symtabstack_pop(self->symbol_table_stack));

				p_raise_error(
					self, param_token,
					"Error: Parameter redefinition of '%s'.", param_id
				);

				return NULL;
			}

			param_symbol = new(Symbol, param_id, fun_symbol_table, DEF_TYPE_VARIABLE);
			symbol_table_define(fun_symbol_table, param_id, param_symbol);

			param_symbols = reallocate(param_symbols, sizeof(void *), ++param_count);
			param_symbols[param_count - 1] = param_symbol;
		}
	}

	const struct Token *rparen_token = NULL;

	if(P_MATCH(self, TOKEN_TYPE_RPAREN)) {
		rparen_token = CUR_TOKEN(self);
		p_advance(self);
	}
	else {
		delete((void *) symtabstack_pop(self->symbol_table_stack));
		free(param_symbols);

		p_raise_error(
			self, lparen_token,
			"SyntaxError: Unmatched '(', expected corresponding ')'."
		);
		return NULL;
	}


	// define the fun's symbol to allow recursion
	
	const void *fun_symbol = new(Symbol, fun_id, parent_symbol_table, def_type);
	symbol_table_define(parent_symbol_table, fun_id, fun_symbol);

	// void *fun_declaration = new(
	// 	FunDecl, fun_symbol, fun_symbol_table,
	// 	param_count, param_symbols, NULL
	// );
	// symbol_set_attr(fun_symbol, SYM_ATTR_KEY_AST, fun_declaration);


	const struct Token *lbrace_token = NULL;

	self->parsing_fun = true;

	if(P_MATCH(self, TOKEN_TYPE_LCURLY_BRACE)) {
		lbrace_token = CUR_TOKEN(self);
		p_advance(self);
	}
	else {
		delete((void *) symtabstack_pop(self->symbol_table_stack));
		free(param_symbols);

		p_raise_error(
			self, rparen_token,
			"SyntaxError: Expected '{' after ')'."
		);
		return NULL;
	}

	uint32_t decl_count = 0;
	void **declarations = NULL;

	#define FUN_FREE(self, decls, count) \
		do { \
			for(uint32_t i = 0; i < count; i++) \
				delete(decls[i]); \
			free(decls); 		\
			free(param_symbols);\
			\
			delete((void *) symtabstack_pop(self->symbol_table_stack)); \
		} while(0)


	while(!p_match_any(self, 2, TOKEN_TYPE_RCURLY_BRACE, TOKEN_TYPE_EOF)) {
		void *decl = declaration(self);

		if(NULL == decl) {
			FUN_FREE(self, declarations, decl_count);
			return NULL;
		}

		declarations = reallocate(declarations, sizeof(void *), ++decl_count);
		declarations[decl_count - 1] = decl;
	}

	if(!p_consume(self, TOKEN_TYPE_RCURLY_BRACE)) {
		FUN_FREE(self, declarations, decl_count);

		p_raise_error(
			self, lbrace_token,
			"SyntaxError: Expected corresponding '}' for '{'."
		);

		return NULL;
	}

	declarations = reallocate(declarations, sizeof(void *), ++decl_count);
	declarations[decl_count - 1] = NULL;

	void *fun_declaration = new(
		FunDecl, fun_symbol, fun_symbol_table,
		param_count, param_symbols, declarations
	);

	// fun_decl_set_declarations(fun_declaration, declarations);

	symtabstack_pop(self->symbol_table_stack);	// pop fun's symbol_table

	self->parsing_fun = false;

	return fun_declaration;
}


//	class_decl	->  "class" IDENTIFIER "{" var_decl* "} ;

void *class_decl(struct Parser *self) {
	const struct Token *class_token = CUR_TOKEN(self);
	p_advance(self);

	const struct Token *class_id_token = NULL;

	if(P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
		class_id_token = CUR_TOKEN(self);
		p_advance(self);
	}
	else {
		p_raise_error(
			self, class_token,
			"SyntaxError: Expected class's name."
		);

		return NULL;
	}

	const struct Token *lbrace_token = NULL;

	if(P_MATCH(self, TOKEN_TYPE_LCURLY_BRACE)) {
		lbrace_token = CUR_TOKEN(self);
		p_advance(self);
	}
	else {
		p_raise_error(
			self, class_token,
			"SyntaxError: Expected class' body."
		);

		return NULL;
	}

	const void *parent_symbol_table = symtabstack_get_local_table(self->symbol_table_stack);

	// parsing class' body
	void *class_symbol_table = (void *) symtabstack_push(self->symbol_table_stack);


	char *it_id = allocate(sizeof(char), 2 + 1);
	strcpy(it_id, "it");

	void *it_symbol = new(Symbol, it_id, class_symbol_table, DEF_TYPE_CUR_INSTANCE_POINTER);
	symbol_table_define(class_symbol_table, it_id, it_symbol);


	uint32_t property_count = 0;
	void **properties = NULL;

	void *methods = NULL;

	while(!p_match_any(self, 2, TOKEN_TYPE_RCURLY_BRACE, TOKEN_TYPE_EOF)) {
		if(P_MATCH(self, TOKEN_TYPE_VAR) || P_MATCH(self, TOKEN_TYPE_VAL)) {
			
			const struct VarDecl *vardecl = NULL;

			if(P_MATCH(self, TOKEN_TYPE_VAR)) {
				vardecl = var_decl(self, DEF_TYPE_MUTABLE_PROPERTY);
			}
			else if(P_MATCH(self, TOKEN_TYPE_VAL)) {
				vardecl = var_decl(self, DEF_TYPE_IMMUTABLE_PROPERTY);
			}
			
			int last_count = property_count;
			int count = 0;

			for(int i = 0; NULL != vardecl->symbols[i]; i++) {
				count++;
			}
			
			property_count += count;
			properties = reallocate(properties, sizeof(void *), property_count);

			for(int i = last_count, j = 0; i < property_count; i++, j++) {
				properties[i] = vardecl->symbols[j];
			}

			delete((void *) vardecl);
		}
		else if(P_MATCH(self, TOKEN_TYPE_FUN)) {
			const struct FunDecl *fundecl = fun_decl(self, DEF_TYPE_METHOD);

			// it's wrong fix this, use a list instead of hashtable
			if(NULL == methods) {
				methods = new(HashTable);
			}

			// want to support method polymorphism
			hashtable_put(methods, SYMBOL_GET_ID(fundecl->symbol), fundecl);
		}
		else {
			// free and return

			p_raise_error(
				self, CUR_TOKEN(self),
				"SyntaxError: Garbage code."
			);
			p_advance(self);
			return NULL;
		}
	}

	symtabstack_pop(self->symbol_table_stack);	// class_symbol_table


	const struct Token *rbrace_token = NULL;

	if(!p_consume(self, TOKEN_TYPE_RCURLY_BRACE)) {
		p_raise_error(
			self, lbrace_token,
			"SyntaxError: Unmatched '{', expected corressponding '}'."
		);

		return NULL;
	}

	char *class_id = allocate(sizeof(char), class_id_token->length + 1);
	strncpy(class_id, class_id_token->start, class_id_token->length);

	void *class_symbol = new(Symbol, class_id, parent_symbol_table, DEF_TYPE_CLASS);
	symbol_table_define(parent_symbol_table, class_id, class_symbol);

	return new(ClassDecl, class_symbol, class_symbol_table, property_count, properties, methods);
}









// stmt -> expr_stmt | if_stmt | when_stmt | loop_stmt | repeat_stmt | block_stmt

void *stmt(struct Parser *self) {
	// printf("stmt()\n");

	if(P_MATCH(self, TOKEN_TYPE_IF))
		return if_stmt(self);

	if(P_MATCH(self, TOKEN_TYPE_WHEN))
		return when_stmt(self);

	else if(P_MATCH(self, TOKEN_TYPE_LOOP))
		return loop_stmt(self);

	else if(P_MATCH(self, TOKEN_TYPE_REPEAT))
		return repeat_stmt(self);

	else if(P_MATCH(self, TOKEN_TYPE_LCURLY_BRACE))
		return block_stmt(self);
	
	else if(P_MATCH(self, TOKEN_TYPE_BREAK))
		return break_stmt(self);
	
	else if(P_MATCH(self, TOKEN_TYPE_CONTINUE))
		return continue_stmt(self);
	
	else if(P_MATCH(self, TOKEN_TYPE_RET))
		return ret_stmt(self);

	// else if(P_MATCH(self, TOKEN_TYPE_PRINT))
	// 	return print_stmt(self);

	else
		return expr_stmt(self);
}



// expr_stmt -> expr ";" ;

void *expr_stmt(struct Parser *self) {
	const struct Token *token = CUR_TOKEN(self);

	void *expression = expr(self);

	if(NULL == expression) {
		return NULL;
	}

	if(!p_consume(self, TOKEN_TYPE_SEMICOLON)) {
		delete(expression);

		p_raise_error(
			self, token,
			"SyntaxError: Expected ';' after this expression."
		);
		return NULL;
	}

	return new(ExprStmt, expression);
}


// if_stmt -> "if" "(" expr ")" stmt ( "else" stmt )? ;

void *if_stmt(struct Parser *self) {
	const struct Token *if_token = CUR_TOKEN(self);
	p_advance(self);

	const struct Token *lparen_token = NULL;

	if(P_MATCH(self, TOKEN_TYPE_LPAREN)) {
		lparen_token = CUR_TOKEN(self);
		p_advance(self);
	}
	else {
		p_raise_error(
			self, if_token,
			"SyntaxError: Expected some expression in a pair of parenthesis after if."
		);
		return NULL;
	}

	void *expression = expr(self);

	if(NULL == expression) {
		p_raise_error(
			self, lparen_token,
			"SyntaxError: Expected some expression after '('."
		);
		return NULL;
	}

	if(!p_consume(self, TOKEN_TYPE_RPAREN)) {
		delete(expression);

		p_raise_error(
			self, lparen_token,
			"SyntaxError: Expected corresponding ')' for '('."
		);

		return NULL;
	}

	void *if_block = declaration(self);

	if(NULL == if_block) {
		delete(expression);

		p_raise_error(
			self, if_token,
			"SyntaxError: Expected some statement for if."
		);

		return NULL;
	}

	void *else_block = NULL;

	if(P_MATCH(self, TOKEN_TYPE_ELSE)) {
		const struct Token *else_token = CUR_TOKEN(self);
		p_advance(self);

		else_block = stmt(self);

		if(NULL == else_block) {
			delete(if_block);
			delete(expression);

			p_raise_error(
				self, else_token,
				"SyntaxError: Expected some statement for else."
			);

			return NULL;
		}
	}

	return new(IfStmt, expression, if_block, else_block);
}


/*
when_stmt	->  "when" "(" expr ")" "{"
					(term ("," term)* "->" stmt)+
					("else" -> stmt)?
				"}" ;
*/

void *when_case(struct Parser *self);

void *when_stmt(struct Parser *self) {
	// printf("when_stmt()\n");

	const struct Token *when_token = CUR_TOKEN(self);
	p_advance(self);

	const struct Token *lparen_token = NULL;
	const struct Token *rparen_token = NULL;
	const struct Token *lbrace_token = NULL;

	if(P_MATCH(self, TOKEN_TYPE_LPAREN)) {
		lparen_token = CUR_TOKEN(self);
		p_advance(self);
	} else {
		p_raise_error(
			self, when_token,
			"SyntaxError: Expected some expression in a pair of parenthesis after when."
		);

		return NULL;
	}

	void *condition_expr = expr(self);

	if(NULL == condition_expr) {
		p_raise_error(
			self, lparen_token,
			"SyntaxError: Expected some expression enclosed in parenthesis after when."
		);

		return NULL;
	}

	if(P_MATCH(self, TOKEN_TYPE_RPAREN)) {
		rparen_token = CUR_TOKEN(self);
		p_advance(self);
	}
	else {
		delete(condition_expr);

		p_raise_error(
			self, lparen_token,
			"SyntaxError: Expected corresponding ')' for '('."
		);

		return NULL;
	}

	if(P_MATCH(self, TOKEN_TYPE_LCURLY_BRACE)) {
		lbrace_token = CUR_TOKEN(self);
		p_advance(self);
	}
	else {
		delete(condition_expr);
		p_raise_error(
			self, rparen_token,
			"SyntaxError: Expected some case statements enclosed in curly braces after when's ')'."
		);

		return NULL;
	}

	void *parsed_when_case = when_case(self);

	if(NULL == parsed_when_case) {
		delete(condition_expr);

		p_raise_error(
			self, when_token,
			"SyntaxError: Expected some case statements after when's '{'."
		);

		return NULL;
	}

	uint32_t case_count = 0;
	void **cases = allocate(sizeof(void *), ++case_count);

	void *else_stmt = NULL;

	cases[case_count - 1] = parsed_when_case;

	while(!p_match_any(self, 3, TOKEN_TYPE_ELSE, TOKEN_TYPE_RCURLY_BRACE, TOKEN_TYPE_EOF)) {
		parsed_when_case = when_case(self);
		
		if(NULL == parsed_when_case)
			break;
		
		cases = reallocate(cases, sizeof(void *), ++case_count);
		cases[case_count - 1] = parsed_when_case;
	}

	cases = reallocate(cases, sizeof(void *), ++case_count);
	cases[case_count - 1] = NULL;


	if(P_MATCH(self, TOKEN_TYPE_ELSE)) {
		const struct Token *else_token = CUR_TOKEN(self);
		p_advance(self);

		const struct Token *arrow_token = NULL;

		if(P_MATCH(self, TOKEN_TYPE_DASH_ARROW)) {
			arrow_token = CUR_TOKEN(self);
			p_advance(self);
		}
		else {
			p_raise_error(
				self, else_token,
				"SyntaxError: Expected '->' after 'else'."
			);

			for(uint32_t i = 0; NULL != cases[i]; i++)
				delete(cases[i]);

			return NULL;
		}

		else_stmt = stmt(self);

		if(NULL == else_stmt) {
			p_raise_error(
				self, arrow_token,
				"SyntaxError: Expected some statement after '->'."
			);

			for(uint32_t i = 0; NULL != cases[i]; i++)
				delete(cases[i]);

			return NULL;
		}
	}

	if(!p_consume(self, TOKEN_TYPE_RCURLY_BRACE)) {
		p_raise_error(
			self, lbrace_token,
			"SyntaxError: Expected corresponding '}' for '{'."
		);

		delete(condition_expr);

		for(uint32_t i = 0; NULL != cases[i]; i++)
			delete(cases[i]);
		
		if(NULL != else_stmt)
			delete(else_stmt);

		return NULL;
	}

	// printf("ret when_stmt\n");
	return new(WhenStmt, condition_expr, cases, else_stmt);
}



// when_case -> term ("," term)* "->" stmt

void *when_case(struct Parser *self) {

	#define FREE_EXPRS(exprs) \
		do { \
			for(uint32_t i = 0; exprs[i] != NULL; i++) \
				delete(exprs[i]);\
		} while(0) \

	void **case_exprs = NULL;
	uint32_t expr_count = 0;

	void *case_expr = term(self);

	if(NULL == case_expr)
		return NULL;
	
	case_exprs = reallocate(case_exprs, sizeof(void *), ++expr_count);
	case_exprs[expr_count - 1] = case_expr;

	const struct Token *comma_token = NULL;
	const struct Token *last_token = CUR_TOKEN(self);

	while(P_MATCH(self, TOKEN_TYPE_COMMA)) {
		comma_token = CUR_TOKEN(self);
		p_advance(self);

		last_token = CUR_TOKEN(self);
		case_expr = term(self);

		if(NULL == case_expr) {
			// got to deallocate exprs
			FREE_EXPRS(case_exprs);

			p_raise_error(
				self, comma_token,
				"SyntaxError: Expected some expression after ','."
			);

			return NULL;
		}

		case_exprs = reallocate(case_exprs, sizeof(void *), ++expr_count);
		case_exprs[expr_count - 1] = case_expr;
	}

	case_exprs = reallocate(case_exprs, sizeof(void *), ++expr_count);
	case_exprs[expr_count - 1] = NULL;


	const struct Token *arrow_token = NULL;

	if(P_MATCH(self, TOKEN_TYPE_DASH_ARROW)) {
		arrow_token = CUR_TOKEN(self);
		p_advance(self);
	}
	else {
		FREE_EXPRS(case_exprs);

		p_raise_error(
			self, last_token,
			"SyntaxError: Expected '->' after expression."
		);

		return NULL;
	}

	void *case_stmt = stmt(self);

	if(NULL == case_stmt) {
		FREE_EXPRS(case_exprs);

		p_raise_error(
			self, arrow_token,
			"SyntaxError: Expected some statement after '->'."
		);

		return NULL;
	}

	return new(WhenCase, case_exprs, case_stmt);

	#undef FREE_EXPRS
}






// loop_stmt -> 'loop' '(' (IDENTIFIER ':')? term ('->' | '=>') term (';' term)? ')' stmt	|
//				'loop' '(' expr ')' stmt ;

void *loop_stmt(struct Parser *self) {
	const struct Token *loop_token = CUR_TOKEN(self);
	p_advance(self);

	const struct Token *lparen_token = NULL;

	if(P_MATCH(self, TOKEN_TYPE_LPAREN)) {
		lparen_token = CUR_TOKEN(self);
		p_advance(self);
	}
	else {
		p_raise_error(
			self, loop_token,
			"SyntaxError: Expected '(' after loop."
		);
		return NULL;
	}

	const struct Token *arrow_token = NULL;
	const struct Token *id_token = NULL;

	enum LoopStmtType loop_type;
	
	// if(P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
	// 	id_token = CUR_TOKEN(self);
	// 	p_advance(self);
	// 	if(!p_consume(self, TOKEN_TYPE_COLON)) {
	// 		p_raise_error(
	// 			self, id_token,
	// 			"SyntaxError: Expected ':' after '%.*s'.", id_token->length, id_token->start
	// 		);
	// 		return NULL;
	// 	}
	// }

	void *start_expr = expr(self);
	void *end_expr = NULL;
	void *step_expr = NULL;

	
	if(NULL == start_expr) {
		p_raise_error(
			self, lparen_token,
			"SyntaxError: Expected some expression after '('."
		);
		return NULL;
	}

	if(P_MATCH(self, TOKEN_TYPE_RPAREN)) {
		p_advance(self);

		// if(NULL != id_token) {
		// 	p_raise_error(
		// 		self, id_token,
		// 		"SyntaxError: Unexpected '%.*s'.", id_token->length, id_token->start
		// 	);
		// 	return NULL;
		// }

		loop_type = LOOP_STMT_TYPE_CONDITIONAL;
	}
	else {
		loop_type = LOOP_STMT_TYPE_RANGED;

		if(p_match_any(self, 2, TOKEN_TYPE_DASH_ARROW, TOKEN_TYPE_EQUAL_ARROW)) {
			arrow_token = CUR_TOKEN(self);
			p_advance(self);
		}
		else {
			delete(start_expr);

			p_raise_error(
				self, loop_token,
				"SyntaxError: Expected '->' or '=>' after expression for loop."
			);

			return NULL;
		}

		end_expr = term(self);

		if(NULL == end_expr) {
			delete(start_expr);

			// p_raise_error(
			// 	self, arrow_token,
			// 	"SyntaxError: Expected some expression after arrow."
			// );

			return NULL;
		}

		if(P_MATCH(self, TOKEN_TYPE_SEMICOLON)) {
			const struct Token *semicolon = CUR_TOKEN(self);
			p_advance(self);

			step_expr = term(self);
			
			if(NULL == step_expr) {
				delete(start_expr);
				delete(end_expr);

				p_raise_error(
					self, semicolon,
					"SyntaxError: Expected some step value after ';'."
				);

				return NULL;
			}
		}

		if(P_MATCH(self, TOKEN_TYPE_RPAREN)) {
			p_advance(self);
		}
		else {
			delete(start_expr);
			delete(end_expr);

			if(NULL != step_expr)
				delete(step_expr);

			p_raise_error(
				self, lparen_token,
				"SyntaxError: Expected corresponding ')' for '('."
			);

			return NULL;
		}
	}


	const void *loop_symbol_table = symtabstack_push(self->symbol_table_stack);

	// if(LOOP_STMT_TYPE_RANGED == loop_type) {
	// 	char *loop_it_id = NULL;
	// 	if(NULL != id_token) {
	// 		loop_it_id = allocate(sizeof(char), id_token->length + 1);
	// 		strncpy(loop_it_id, id_token->start, id_token->length);
	// 	}
	// 	else {
	// 		const char *loop_iterator = "it";
	// 		loop_it_id = allocate(sizeof(char), strlen(loop_iterator) + 1);
	// 		strncpy(loop_it_id, loop_iterator, strlen(loop_iterator));
	// 	}
	// 	void *symbol = new(Symbol, loop_it_id, loop_symbol_table, DEF_TYPE_VARIABLE);
	// 	symbol_table_define(loop_symbol_table, loop_it_id, symbol);
	// }


	#define LOOP_STMT_FREE(stmts, count) \
		do { \
			for(uint32_t i = 0; i < count; i++) \
				delete(stmts[i]); \
				\
			free(stmts); \
			symtabstack_pop(self->symbol_table_stack);	\
		} while(0)


	uint32_t child_stmts_count = 0;
	void **child_stmts = NULL;

	self->parsing_loop = true;

	if(P_MATCH(self, TOKEN_TYPE_LCURLY_BRACE)) {
		const struct Token *lbrace_token = CUR_TOKEN(self);
		p_advance(self);

		if(loop_type == LOOP_STMT_TYPE_RANGED) {
			if(P_MATCH(self, TOKEN_TYPE_IDENTIFIER) &&
				P_PEEK(self, TOKEN_TYPE_DASH_ARROW)) {

				id_token = CUR_TOKEN(self);
				p_advance(self);

				p_advance(self);
			}


			char *loop_iterator_id = NULL;

			if(NULL != id_token) {
				loop_iterator_id = allocate(sizeof(char), id_token->length + 1);
				strncpy(loop_iterator_id, id_token->start, id_token->length);
			}
			else {
				const char *it = "it";

				loop_iterator_id = allocate(sizeof(char), strlen(it) + 1);
				strncpy(loop_iterator_id, it, strlen(it));
			}

			void *symbol = new(Symbol, loop_iterator_id, loop_symbol_table, DEF_TYPE_VARIABLE);
			symbol_table_define(loop_symbol_table, loop_iterator_id, symbol);

		}
		else {
			if(P_MATCH(self, TOKEN_TYPE_IDENTIFIER) &&
				P_PEEK(self, TOKEN_TYPE_DASH_ARROW)) {

				id_token = CUR_TOKEN(self);

				delete(start_expr);
				symtabstack_pop(self->symbol_table_stack);	// loop_symbol_table

				p_raise_error(
					self, id_token,
					"SyntaxError: Unexpected '%.*s'.", id_token->length, id_token->start
				);

				return NULL;
			}
		}



		while(!p_match_any(self, 2, TOKEN_TYPE_EOF, TOKEN_TYPE_RCURLY_BRACE)) {
			void *child_stmt = declaration(self);

			if(NULL == child_stmt) {
				LOOP_STMT_FREE(child_stmts, child_stmts_count);
				return NULL;
			}

			child_stmts = reallocate(child_stmts, sizeof(void *), ++child_stmts_count);
			child_stmts[child_stmts_count - 1] = child_stmt;
		}

		if(!p_consume(self, TOKEN_TYPE_RCURLY_BRACE)) {
			p_raise_error(
				self, lbrace_token,
				"SyntaxError: Unmatched '{' expected corresponding '}'."
			);

			LOOP_STMT_FREE(child_stmts, child_stmts_count);

			return NULL;
		}

		child_stmts = reallocate(child_stmts, sizeof(void *), ++child_stmts_count);
		child_stmts[child_stmts_count - 1] = NULL;
	}
	else {

		if(loop_type == LOOP_STMT_TYPE_RANGED) {
			const char *loop_iterator = "it";

			char *loop_it_id = allocate(sizeof(char), strlen(loop_iterator) + 1);
			strncpy(loop_it_id, loop_iterator, strlen(loop_iterator));

			void *symbol = new(Symbol, loop_it_id, loop_symbol_table, DEF_TYPE_VARIABLE);
			symbol_table_define(loop_symbol_table, loop_it_id, symbol);
		}

		void *child_stmt = declaration(self);

		if(NULL == child_stmt) {
			symtabstack_pop(self->symbol_table_stack);	// loop_symbol_table
			return NULL;
		}

		child_stmts = allocate(sizeof(void *), 2);
		child_stmts[0] = child_stmt;
		child_stmts[1] = NULL;
	}

	symtabstack_pop(self->symbol_table_stack);	// loop_symbol_table

	self->parsing_loop = false;

	return new(
		LoopStmt, loop_symbol_table, loop_type,
		id_token, start_expr, arrow_token, end_expr, step_expr,
		child_stmts
	);
}





// repeat_stmt -> "repeat" "(" (IDENTIFIER ':')? term ")" stmt ;

void *repeat_stmt(struct Parser *self) {
	const struct Token *repeat_token = CUR_TOKEN(self);
	p_advance(self);

	const struct Token *lparen_token = NULL;

	if(P_MATCH(self, TOKEN_TYPE_LPAREN)) {
		lparen_token = CUR_TOKEN(self);
		p_advance(self);
	}
	else {
		p_raise_error(
			self, repeat_token,
			"SyntaxError: Expected '(' after repeat."
		);		
		return NULL;
	}

	const struct Token *id_token = NULL;

	void *expression = term(self);

	if(NULL == expression) {
		p_raise_error(
			self, lparen_token,
			"SyntaxError: Expected some expression after '('."
		);
		return NULL;
	}

	const struct Token *cur = CUR_TOKEN(self);

	if((!P_MATCH(self, TOKEN_TYPE_EOF)) &&
		p_match_any(self, 8, TOKEN_TYPE_LOGICAL_AND, TOKEN_TYPE_LOGICAL_OR, TOKEN_TYPE_EQUAL_EQUAL, TOKEN_TYPE_NOT_EQUAL,
							TOKEN_TYPE_LESS_THAN, TOKEN_TYPE_LESS_THAN_EQUALS, TOKEN_TYPE_GREATER_THAN, TOKEN_TYPE_GREATER_THAN_EQUALS)
	) {
		delete(expression);
		p_raise_error(
			self, cur,
			"Error: Unexpected '%.*s' in repeat's expression.", cur->length, cur->start
		);
		return NULL;
	}

	if(!P_MATCH(self, TOKEN_TYPE_RPAREN)) {
		delete(expression);

		p_raise_error(
			self, lparen_token,
			"Expected corresponding ')' for '('."
		);

		return NULL;
	}
	else {
		p_advance(self);
	}

	const void *repeat_symbol_table = symtabstack_push(self->symbol_table_stack);

	#define REPEAT_STMT_FREE(stmts, count) \
		do { \
			for(uint32_t i = 0; i < count; i++) \
				delete(stmts[i]); \
				\
			free(stmts); \
			symtabstack_pop(self->symbol_table_stack);	\
		} while(0)


	uint32_t child_stmts_count = 0;
	void **child_stmts = NULL;

	self->parsing_loop = true;

	if(P_MATCH(self, TOKEN_TYPE_LCURLY_BRACE)) {
		const struct Token *lbrace_token = CUR_TOKEN(self);
		p_advance(self);

		if(P_MATCH(self, TOKEN_TYPE_IDENTIFIER) &&
			P_PEEK(self, TOKEN_TYPE_DASH_ARROW)) {

			id_token = CUR_TOKEN(self);
			p_advance(self);

			p_advance(self);
		}

		char *repeat_iterator_id = NULL;

		if(NULL != id_token) {
			repeat_iterator_id = allocate(sizeof(char), id_token->length + 1);
			strncpy(repeat_iterator_id, id_token->start, id_token->length);
		}
		else {
			const char *it = "it";

			repeat_iterator_id = allocate(sizeof(char), strlen(it) + 1);
			strncpy(repeat_iterator_id, it, strlen(it));
		}

		void *symbol = new(Symbol, repeat_iterator_id, repeat_symbol_table, DEF_TYPE_VARIABLE);
		symbol_table_define(repeat_symbol_table, repeat_iterator_id, symbol);



		while(!p_match_any(self, 2, TOKEN_TYPE_EOF, TOKEN_TYPE_RCURLY_BRACE)) {
			void *child_stmt = declaration(self);

			if(NULL == child_stmt) {
				REPEAT_STMT_FREE(child_stmts, child_stmts_count);

				if(child_stmts_count == 0) {
					p_raise_error(
						self, lbrace_token,
						"SyntaxError: Expected some statements after '{'."
					);
				}

				return NULL;
			}

			child_stmts = reallocate(child_stmts, sizeof(void *), ++child_stmts_count);
			child_stmts[child_stmts_count - 1] = child_stmt;
		}

		if(!p_consume(self, TOKEN_TYPE_RCURLY_BRACE)) {
			p_raise_error(
				self, lbrace_token,
				"SyntaxError: Expected corresponding '}' for '{'."
			);

			REPEAT_STMT_FREE(child_stmts, child_stmts_count);

			return NULL;
		}

		child_stmts = reallocate(child_stmts, sizeof(void *), ++child_stmts_count);
		child_stmts[child_stmts_count - 1] = NULL;
	}
	else {

		const char *it = "it";

		char *repeat_iterator_id = allocate(sizeof(char), strlen(it) + 1);
		strncpy(repeat_iterator_id, it, strlen(it));

		void *symbol = new(Symbol, repeat_iterator_id, repeat_symbol_table, DEF_TYPE_VARIABLE);
		symbol_table_define(repeat_symbol_table, repeat_iterator_id, symbol);


		void *child_stmt = declaration(self);

		if(NULL == child_stmt) {
			symtabstack_pop(self->symbol_table_stack);	// repeat_symbol_table

			p_raise_error(
				self, repeat_token,
				"SyntaxError: Expected some child statement for repeat."
			);

			return NULL;
		}

		child_stmts = allocate(sizeof(void *), 2);
		child_stmts[0] = child_stmt;
		child_stmts[1] = NULL;
	}

	symtabstack_pop(self->symbol_table_stack);	// repeat_symbol_table

	self->parsing_loop = false;

	return new(RepeatStmt, repeat_symbol_table, id_token, expression, child_stmts);

	#undef REPEAT_STMT_FREE
}



// print_stmt -> "print" expr ;
// void *print_stmt(struct Parser *self) {
// 	struct Token *print_token = CUR_TOKEN(self);
// 	p_advance(self);
// 	void *expression = expr(self);
// 	if(NULL == expression) {
// 		// p_raise_error(
// 		// 	self, print_token,
// 		// 	"SyntaxError: Expected some expression to print."
// 		// );
// 		return NULL;
// 	}
// 	if(!p_consume(self, TOKEN_TYPE_SEMICOLON)) {
// 		delete(expression);
// 		p_raise_error(
// 			self, print_token,
// 			"SyntaxError: Expected ';' after print statement."
// 		);
// 		return NULL;
// 	}
// 	return new(PrintStmt, expression);
// }



// block_stmt -> "{" declaration* "}" ;

void *block_stmt(struct Parser *self) {
	// printf("block_stmt()\n");

	#define BLOCK_STMT_FREE \
		do { \
			for(uint32_t i = 0; i < child_stmts_count; i++) \
				delete(child_stmts[i]); \
				\
			free(child_stmts); \
			symtabstack_pop(self->symbol_table_stack); \
		} while(0)


	const struct Token *lcurlybrace = CUR_TOKEN(self);
	p_advance(self);


	const void *block_stmt_symbol_table = symtabstack_push(self->symbol_table_stack);


	uint32_t child_stmts_count = 0;
	void **child_stmts = NULL;

	while(!p_match_any(self, 2, TOKEN_TYPE_RCURLY_BRACE, TOKEN_TYPE_EOF)) {
		void *child_stmt = declaration(self);

		if(NULL == child_stmt) {
			BLOCK_STMT_FREE;
			return NULL;
		}

		child_stmts = reallocate(child_stmts, sizeof(void *), ++child_stmts_count);
		child_stmts[child_stmts_count - 1] = child_stmt;
	}

	if(!p_consume(self, TOKEN_TYPE_RCURLY_BRACE)) {
		BLOCK_STMT_FREE;

		p_raise_error(
			self, lcurlybrace,
			"SyntaxError: Expected corresponding '}' for '{'."
		);
		return NULL;
	}

	child_stmts = reallocate(child_stmts, sizeof(void *), ++child_stmts_count);
	child_stmts[child_stmts_count - 1] = NULL;


	symtabstack_pop(self->symbol_table_stack);	// block_stmt_symbol_table


	return new(BlockStmt, block_stmt_symbol_table, child_stmts);

	#undef BLOCK_STMT_FREE
}







// break_stmt -> 'break' ';' ; 

void *break_stmt(struct Parser *self) {
	const struct Token *break_token = CUR_TOKEN(self);
	p_advance(self);

	if(!self->parsing_loop) {
		p_raise_error(
			self, break_token,
			"Error: Break can be used only within a loop."
		);
		return NULL;		
	}

	const struct Token *semicolon_token = NULL;

	if(!P_MATCH(self, TOKEN_TYPE_SEMICOLON)) {
		p_raise_error(
			self, break_token,
			"SyntaxError: Expected ';' afterwards."
		);
		return NULL;
	}
	else {
		semicolon_token = CUR_TOKEN(self);
		p_advance(self);
	}

	return new(BreakStmt, break_token, semicolon_token);
}


// continue_stmt -> 'continue' ';' ;

void *continue_stmt(struct Parser *self) {
	const struct Token *continue_token = CUR_TOKEN(self);
	p_advance(self);

	if(!self->parsing_loop) {
		p_raise_error(
			self, continue_token,
			"Error: Continue can be used only within a loop."
		);
		return NULL;
	}

	const struct Token *semicolon_token = NULL;

	if(!P_MATCH(self, TOKEN_TYPE_SEMICOLON)) {
		p_raise_error(
			self, continue_token,
			"SyntaxError: Expected ';' afterwards."
		);
		return NULL;
	}
	else {
		semicolon_token = CUR_TOKEN(self);
		p_advance(self);
	}

	return new(ContinueStmt, continue_token, semicolon_token);
}




// ret_stmt -> "ret" expr? ";" ;

void *ret_stmt(struct Parser *self) {
	const struct Token *ret_token = CUR_TOKEN(self);
	p_advance(self);

	if(!self->parsing_fun) {
		p_raise_error(
			self, ret_token,
			"Error: Return can be used only within a funtion or method."
		);
		return NULL;
	}

	void *ret_expr = NULL;

	if(!P_MATCH(self, TOKEN_TYPE_SEMICOLON)) {
		ret_expr = expr(self);

		if(NULL == ret_expr) {
			return NULL;
		}
	}

	if(!p_consume(self, TOKEN_TYPE_SEMICOLON)) {
		p_raise_error(
			self, ret_token,
			"SyntaxError: Expected ';' to end ret statement."
		);

		if(NULL != ret_expr)
			delete(ret_expr);

		return NULL;
	}

	return new(RetStmt, ret_expr);
}









/*
	expr		->  assignment |
					logical_or ;

	assignment	>	molecule ('=' | '+=' | '-=' | '*=' | '/=' | '%=') expr  ;
*/

// v1.0
/* void *expr(struct Parser *self) {
	void *left_expr = logical_or(self);

	if(NULL == left_expr)
		return NULL;

	if(
		p_match_any(
			self, 6,
			TOKEN_TYPE_EQUAL, TOKEN_TYPE_PLUS_EQUAL, TOKEN_TYPE_MINUS_EQUAL,
			TOKEN_TYPE_STAR_EQUAL, TOKEN_TYPE_SLASH_EQUAL, TOKEN_TYPE_PERCENT_EQUAL
		)
	) {
		const struct Token *equal_token = CUR_TOKEN(self);
		p_advance(self);

		if(!IS_IDENTIFIER_EXPR(left_expr) && !IS_LIST_SUBSCRIPT_EXPR(left_expr)) {
			p_raise_error(
				self, equal_token,
				"Error: Invalid lvalue before '%.*s'.", equal_token->length, equal_token->start
			);

			delete(left_expr);
			return NULL;
		}

		const void *id_symbol;
		const struct Token *id_token;

		if(IS_IDENTIFIER_EXPR(left_expr)) {
			id_symbol = IDENTIFIER_EXPR_GET_SYMBOL(left_expr);
			id_token = IDENTIFIER_EXPR_GET_ID_TOKEN(left_expr);
			
			if(DEF_TYPE_VARIABLE != SYMBOL_GET_DEF_TYPE(id_symbol)) {
				p_raise_error(
					self, id_token,
					"Error: lvalue must be a mutable variable."
				);
				return NULL;
			}

		} else if(IS_LIST_SUBSCRIPT_EXPR(left_expr)) {
			// id_symbol = LIST_SUBSCRIPT_EXPR_GET_SYMBOL(left_expr);
			// id_token = LIST_SUBSCRIPT_EXPR_GET_ID_TOKEN(left_expr);
		}

		if(DEF_TYPE_VARIABLE != SYMBOL_GET_DEF_TYPE(id_symbol)) {
			p_raise_error(
				self, id_token,
				"Error: lvalue must be a mutable variable."
			);
			return NULL;
		}

		void *right_expr = expr(self);

		if(NULL == right_expr) {
			delete(left_expr);
			return NULL;
		}

		if(IS_IDENTIFIER_EXPR(left_expr)) {
			struct IdentifierExpr *id_expr = (struct IdentifierExpr *) left_expr;

			void *assign_expr = new(
				AssignExpr, LVALUE_VARIABLE,
				IDENTIFIER_EXPR_GET_ID_TOKEN(id_expr),
				equal_token,
				IDENTIFIER_EXPR_GET_SYMBOL(id_expr),
				NULL, right_expr
			);

			delete(id_expr);

			return assign_expr;
		}
		else {

			// struct ListSubscriptExpr *lsexpr = (struct ListSubscriptExpr *) left_expr;

			// void *assign_expr = new(
			// 	AssignExpr, LVALUE_LIST_SUBSCRIPT,
			// 	LIST_SUBSCRIPT_EXPR_GET_ID_TOKEN(lsexpr),
			// 	equal_token,
			// 	LIST_SUBSCRIPT_EXPR_GET_SYMBOL(lsexpr),
			// 	LIST_SUBSCRIPT_EXPR_GET_INDEX_EXPR(lsexpr),
			// 	right_expr
			// );

			// // cuz don't want to delete lsexpr's index_expr
			// free(lsexpr);
			
			// return assign_expr;

			return NULL;
		}
	}

	return left_expr;
}
*/

// v1.1
void *expr(struct Parser *self) {
	void *left_expr = logical_or(self);

	if(NULL == left_expr)
		return NULL;

	if(
		p_match_any(
			self, 6,
			TOKEN_TYPE_EQUAL, TOKEN_TYPE_PLUS_EQUAL, TOKEN_TYPE_MINUS_EQUAL,
			TOKEN_TYPE_STAR_EQUAL, TOKEN_TYPE_SLASH_EQUAL, TOKEN_TYPE_PERCENT_EQUAL
		)
	) {
		const struct Token *equal_token = CUR_TOKEN(self);
		p_advance(self);

		if(
			!IS_IDENTIFIER_EXPR(left_expr) &&
			!IS_LIST_SUBSCRIPT_EXPR(left_expr) &&
			!IS_GET_EXPR(left_expr)
		) {

			p_raise_error_expr(
				self, left_expr,
				"Error: Invalid assignment target."
			);
			delete(left_expr);
			return NULL;
		}

		void *right_expr = expr(self);

		if(IS_IDENTIFIER_EXPR(left_expr)) {
			struct IdentifierExpr *id_expr = left_expr;

			enum DefinitionType def_type = SYMBOL_GET_DEF_TYPE(id_expr->symbol);

			if(DEF_TYPE_VARIABLE != def_type) {
				if(DEF_TYPE_CONSTANT == def_type) {
					p_raise_error(
						self, id_expr->id_token,
						"Error: Assignment to a constant val '%.*s'.",
						id_expr->id_token->length, id_expr->id_token->start
					);

					delete(left_expr);
					return NULL;
				}

				p_raise_error(
					self, id_expr->id_token,
					"Error: lvalue '%.*s', is not a variable.",
					id_expr->id_token->length, id_expr->id_token->start
				);
				delete(left_expr);
				return NULL;
			}

			void *id_assign_expr = new(
				IdentifierAssignExpr,
				id_expr->id_token, id_expr->symbol,
				equal_token, right_expr
			);
			free(id_expr);

			return id_assign_expr;
		}
		else if(IS_LIST_SUBSCRIPT_EXPR(left_expr)) {
			struct ListSubscriptExpr *ls_expr = left_expr;

			if(IS_CALL_EXPR(ls_expr->left_expr)) {
				p_raise_error_expr(
					self, ls_expr,
					"Error: Invalid assignment target."
				);
				delete(left_expr);
				return NULL;				
			}

			void *ls_assign_expr = new(
				ListSubscriptAssignExpr,
				ls_expr->left_expr,
				ls_expr->index_expr,
				equal_token,
				right_expr
			);
			free(ls_expr);
			
			return ls_assign_expr;
		}
		else if(IS_GET_EXPR(left_expr)) {
			return NULL;
		}
	}

	return left_expr;
}



void *parse_binary(struct Parser *self, void *(*parse_fun)(struct Parser *self), uint32_t count, ...) {
	void *left = parse_fun(self);

	if(NULL == left)
		return NULL;
	
	va_list ap;
	va_start(ap, count);

	for(uint32_t i = 0; i < count; i++) {
		const struct TokenType expected = va_arg(ap, const struct TokenType);

		while(P_MATCH(self, expected)) {
			const struct Token *op_token = CUR_TOKEN(self);
			p_advance(self);

			void *right = parse_fun(self);

			if(NULL == right) {
				va_end(ap);
				delete(left);
				return NULL;
			}

			left = new(BinaryExpr, op_token, left, right);
		}
	}

	va_end(ap);

	return left;
}







// logical_or -> logical_and ( "||" logical_and )*

void *logical_or(struct Parser *self) {
	void *left = logical_and(self);

	if(NULL == left)
		return NULL;

	while(P_MATCH(self, TOKEN_TYPE_LOGICAL_OR)) {
		const struct Token *op_token = CUR_TOKEN(self);
		p_advance(self);		

		void *right = logical_and(self);

		if(NULL == right) {
			delete(left);
			return NULL;
		}

		left = new(LogicalExpr, op_token, left, right);
	}

	return left;
}



// logical_and -> equality ("&&" equality )*

void *logical_and(struct Parser *self) {
	void *left = equality(self);

	if(NULL == left)
		return NULL;

	while(P_MATCH(self, TOKEN_TYPE_LOGICAL_AND)) {
		const struct Token *op_token = CUR_TOKEN(self);
		p_advance(self);		

		void *right = equality(self);

		if(NULL == right) {
			delete(left);
			return NULL;
		}

		left = new(LogicalExpr, op_token, left, right);
	}

	return left;
}



// equality -> comparison ( ("==" | "!=") comparison )*

void *equality(struct Parser *self) {
	return parse_binary(
		self, comparison, 2,
		TOKEN_TYPE_EQUAL_EQUAL, TOKEN_TYPE_NOT_EQUAL
	);
}



// comparison -> term  ( ( "<" | "<=" | ">" | ">=" ) term )?

void *comparison(struct Parser *self) {
	void *left = term(self);

	if(NULL == left)
		return NULL;

	if(
		p_match_any(
			self, 4,
			TOKEN_TYPE_LESS_THAN, TOKEN_TYPE_LESS_THAN_EQUALS,
			TOKEN_TYPE_GREATER_THAN, TOKEN_TYPE_GREATER_THAN_EQUALS
		)
	) {
		const struct Token *op_token = CUR_TOKEN(self);
		p_advance(self);

		void *right = term(self);
		left = new(BinaryExpr, op_token, left, right);
	}

	return left;
}



// term -> factor (('+' | '-') factor)*

void *term(struct Parser *self) {
	return parse_binary(
		self, factor, 2,
		TOKEN_TYPE_PLUS, TOKEN_TYPE_MINUS
	);
}



// factor -> unary (('*' | '/' | '%') unary)*

void *factor(struct Parser *self) {
	return parse_binary(
		self, unary, 3,
		TOKEN_TYPE_STAR, TOKEN_TYPE_FORWARD_SLASH, TOKEN_TYPE_PERCENT
	);
}


/*
unary	->	( "++" | "--" ) IDENTIFIER	|
			('!' | '-' | '+')? unary	|
			molecule			;
*/
void *unary(struct Parser *self) {
	/*
	if(p_match_any(self, 2, TOKEN_TYPE_PLUS_PLUS, TOKEN_TYPE_MINUS_MINUS)) {
		const struct Token *op_token = CUR_TOKEN(self);
		p_advance(self);

		const struct Token *id_token = NULL;

		if(P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
			id_token = CUR_TOKEN(self);
			p_advance(self);
		}
		else {
			p_raise_error(
				self, op_token,
				"SyntaxError: Expected identifier after '%.*s'.", op_token->length, op_token->start
			);
			return NULL;
		}

		char *id = allocate(sizeof(char), id_token->length + 1);
		strncpy(id, id_token->start, id_token->length);

		const void *_id_symbol = symtabstack_lookup(self->symbol_table_stack, id);
		free(id);

		if(NULL == _id_symbol) {
			p_raise_error(
				self, id_token,
				"Error: Undefined '%.*s'.", id_token->length, id_token->start 
			);
			return NULL;
		}

		const struct Symbol *id_symbol = _id_symbol;

		if(id_symbol->definition_type != DEF_TYPE_VARIABLE) {
			p_raise_error(
				self, id_token,
				"Error: '%.*s' not defined as a mutable variable.", id_token->length, id_token->start 
			);
			return NULL;
		}
		TODO
		// return new(UnaryExpr, op_token, id_symbol);
	}
	*/

	if(p_match_any(self, 3, TOKEN_TYPE_NOT, TOKEN_TYPE_MINUS, TOKEN_TYPE_PLUS)) {
		const struct Token *op_token = CUR_TOKEN(self);
		p_advance(self);

		void *right = unary(self);
		
		if(NULL == right) {
			p_raise_error(
				self, op_token,
				"SyntaxError: Expected int, float or identifier after '%c'.", op_token->start[0]
			);

			return NULL;
		}

		return new(UnaryExpr, op_token, right);
	}

	return molecule(self);
}




/*
molecule	->  atom							|
				atom ( '(' arguments? ')' )*	|
				atom ( '.' IDENTIFIER )*		|
				atom ( '[' expr ']' )*			;

arguments	->	expr (',' expr)* ;
*/

/*
void *molecule(struct Parser *self) {
	const struct Token *start_token = CUR_TOKEN(self);

	void *left = atom(self);

	if(NULL == left) {
		return NULL;
	}

	if(!P_MATCH(self, TOKEN_TYPE_LPAREN)) {
		return left;
	}


	// we have a '('
	const struct Token *lparen_token = CUR_TOKEN(self);
	p_advance(self);

	// if(!IS_IDENTIFIER_EXPR(left)) {
	// 	if(start_token->token_type.ordinal != TOKEN_TYPE_LPAREN.ordinal) {
	// 		p_raise_error(
	// 			self, start_token,
	// 			"Error: Callee must be a function."
	// 		);
	// 	} else {
	// 		p_raise_error(
	// 			self, lparen_token,
	// 			"Error: Unexpected expression before '('."
	// 		);
	// 	}
	// 	return NULL;
	// }

	// delete(left);


	// const struct Token *id_token = start_token;

	// char *fun_id = allocate(sizeof(char), id_token->length + 1);
	// strncpy(fun_id, id_token->start, id_token->length);

	// const struct Symbol *fun_symbol = symtabstack_lookup(self->symbol_table_stack, fun_id);
	// free(fun_id);

	// if(NULL == fun_symbol) {
	// 	p_raise_error(
	// 		self, id_token,
	// 		"Error: Undefined function '%.*s'.", id_token->length, id_token->start
	// 	);
	// 	return NULL;
	// }

	// uint32_t param_count = 0;
	// if(DEF_TYPE_NATIVE_FUN == fun_symbol->definition_type) {
	// 	const struct FunValue *fun = symbol_get_attr(fun_symbol, SYM_ATTR_KEY_FUN);
	// 	param_count = fun->param_count;
	// }
	// else {
	// 	const struct FunDecl *fun_declaration = symbol_get_attr(fun_symbol, SYM_ATTR_KEY_AST);
	// 	param_count = fun_declaration->param_count;
	// }


	#define CALL_EXPR_FREE(args, count) \
		do { \
			if(NULL != args) { \
				for(uint32_t i = 0; i < count; i++) { \
					if(NULL != args[i])		\
						delete(args[i]); 	\
				} \
			} \
			free(args); \
		} while(0)

	uint32_t arg_count = 0;
	// void **arguments = NULL;
	void **arguments = NULL;

	// if(param_count > 0) {
	// 	arguments = (void **) allocate(sizeof(void *), param_count);
	// }

	if(!p_match_any(self, 2, TOKEN_TYPE_RPAREN, TOKEN_TYPE_EOF)) {
		void *arg = expr(self);

		if(NULL == arg) {
			CALL_EXPR_FREE(arguments, arg_count);
			return NULL;
		}

		arguments = reallocate(arguments, sizeof(void *), ++arg_count);
		arguments[arg_count - 1] = arg;

		// if(arg_count <= param_count) {
		// 	arguments[arg_count - 1] = arg;
		// } else {
		// 	delete(arg);
		// }

		while(P_MATCH(self, TOKEN_TYPE_COMMA)) {
			const struct Token *comma_token = CUR_TOKEN(self);
			p_advance(self);

			arg = expr(self);

			if(NULL == arg) {
				CALL_EXPR_FREE(arguments, arg_count);

				p_raise_error(
					self, comma_token,
					"SyntaxError: Expected expression after ','." 
				);

				return NULL;
			}

			arguments = reallocate(arguments, sizeof(void *), ++arg_count);
			arguments[arg_count - 1] = arg;

			// arg_count++;
			// if(arg_count <= param_count) {
			// 	arguments[arg_count - 1] = arg;
			// } else {
			// 	delete(arg);
			// }
		}
	}

	if(!p_consume(self, TOKEN_TYPE_RPAREN)) {
		CALL_EXPR_FREE(arguments, arg_count);

		p_raise_error(
			self, lparen_token,
			"SyntaxError: Expected corresponding ')' for '('."
		);
		return NULL;
	}

	// if(arg_count < param_count) {
	// 	CALL_EXPR_FREE(arguments, arg_count);

	// 	p_raise_error(
	// 		self, id_token,
	// 		"Error: Too few arguments passed to '%.*s', expected %d argument(s), passed %d.",
	// 		id_token->length, id_token->start, param_count, arg_count
	// 	);
	// 	return NULL;
	// }
	// if(arg_count > param_count) {
	// 	CALL_EXPR_FREE(arguments, param_count);

	// 	p_raise_error(
	// 		self, id_token,
	// 		"Error: Too many arguments passed to '%.*s', expected %d argument(s), passed %d.",
	// 		id_token->length, id_token->start, param_count, arg_count
	// 	);

	// 	return NULL;
	// }

	// return new(CallExpr, id_token, fun_symbol, arg_count, arguments);

	return new(CallExpr, left, arg_count, arguments);
	// return new(CallExpr,  arg_count, arguments);
}
*/


void *molecule(struct Parser *self) {
	const struct Token *start_token = CUR_TOKEN(self);

	void *left = atom(self);

	if(NULL == left) {
		return NULL;
	}

	while(1) {
		if(P_MATCH(self, TOKEN_TYPE_LPAREN)) {
			const struct Token *lparen = CUR_TOKEN(self);
			p_advance(self);

			#define CALL_EXPR_FREE(args, count) \
				do { \
					if(NULL != args) { \
						for(uint32_t i = 0; i < count; i++) { \
							if(NULL != args[i])		\
								delete(args[i]); 	\
						} \
					} \
					free(args); \
				} while(0)

			uint32_t arg_count = 0;
			void **arguments = NULL;

			if(!p_match_any(self, 2, TOKEN_TYPE_RPAREN, TOKEN_TYPE_EOF)) {
				void *arg = expr(self);

				if(NULL == arg) {
					CALL_EXPR_FREE(arguments, arg_count);
					return NULL;
				}

				arguments = reallocate(arguments, sizeof(void *), ++arg_count);
				arguments[arg_count - 1] = arg;

				while(P_MATCH(self, TOKEN_TYPE_COMMA)) {
					const struct Token *comma_token = CUR_TOKEN(self);
					p_advance(self);

					arg = expr(self);

					if(NULL == arg) {
						CALL_EXPR_FREE(arguments, arg_count);

						p_raise_error(
							self, comma_token,
							"SyntaxError: Expected expression after ','." 
						);

						return NULL;
					}

					arguments = reallocate(arguments, sizeof(void *), ++arg_count);
					arguments[arg_count - 1] = arg;
				}
			}

			const struct Token *rparen = NULL;

			if(!P_MATCH(self, TOKEN_TYPE_RPAREN)) {
				CALL_EXPR_FREE(arguments, arg_count);

				p_raise_error(
					self, lparen,
					"SyntaxError: Expected corresponding ')' for '('."
				);
				return NULL;
			}
			else {
				rparen = CUR_TOKEN(self);
				p_advance(self);
			}

			left = new(CallExpr, left, lparen, arg_count, arguments, rparen);
		}
		else if(P_MATCH(self, TOKEN_TYPE_DOT)) {
			const struct Token *dot_token = CUR_TOKEN(self);
			p_advance(self);

			const struct Token *property_token = NULL;

			if(P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
				property_token = CUR_TOKEN(self);
				p_advance(self);
			}
			else {
				delete(left);

				p_raise_error(
					self, dot_token,
					"SyntaxError: Expected some field after '.'."
				);
				return NULL;
			}

			left = new(GetExpr, left, property_token);
		}
		else if(P_MATCH(self, TOKEN_TYPE_LSQUARE_BRACE)) {
			const struct Token *lsbrace = CUR_TOKEN(self);
			p_advance(self);

			void *index_expr = expr(self);

			if(NULL == index_expr) {
				delete(left);
				return NULL;
			}

			const struct Token *rsbrace = NULL;

			if(!P_MATCH(self, TOKEN_TYPE_RSQUARE_BRACE)) {
				p_raise_error(
					self, lsbrace,
					"SyntaxError: Unmatched '[', expected corressponding ']'."
				);
				return NULL;
			}
			else {
				rsbrace = CUR_TOKEN(self);
				p_advance(self);
			}

			left =  new(ListSubscriptExpr, left, lsbrace, index_expr, rsbrace);
		}
		else {
			break;
		}
	}

	return left;
}






/*
atom -> INTEGER | DOUBLE		|
		"true"	| "false"		|
		STRING					|
		IDENTIFIER				|
		"(" expr ")"			|	// group expr
		"[" expr ("," expr)* "]"|	// list expr
		expr "[" expr "]"		;	// subscript expr
*/

void *atom(struct Parser *self) {
	// printf("atom()\n");

	if(P_MATCH(self, TOKEN_TYPE_INTEGER)) {
		const struct Token *literal_token = CUR_TOKEN(self);
		p_advance(self);

		return new(LiteralExpr, literal_token);
	}

	else if(P_MATCH(self, TOKEN_TYPE_DOUBLE)) {
		const struct Token *literal_token = CUR_TOKEN(self);
		p_advance(self);

		return new(LiteralExpr, literal_token);
	}

	else if(P_MATCH(self, TOKEN_TYPE_STRING)) {
		const struct Token *literal_token = CUR_TOKEN(self);
		p_advance(self);

		return new(LiteralExpr, literal_token);
	}

	else if(P_MATCH(self, TOKEN_TYPE_TRUE)) {
		const struct Token *literal_token = CUR_TOKEN(self);
		p_advance(self);

		return new(LiteralExpr, literal_token);
	}

	else if(P_MATCH(self, TOKEN_TYPE_FALSE)) {
		const struct Token *literal_token = CUR_TOKEN(self);
		p_advance(self);

		return new(LiteralExpr, literal_token);
	}

	else if(P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
		const struct Token *id_token = CUR_TOKEN(self);
		p_advance(self);

		char *id = allocate(sizeof(char *), id_token->length + 1);
		strncpy(id, id_token->start, id_token->length);

		const void *id_symbol = symtabstack_lookup(self->symbol_table_stack, id);
		free(id);

		if(NULL == id_symbol) {
			p_raise_error(
				self, id_token,
				"Error: '%.*s' undefined.", id_token->length, id_token->start
			);

			return NULL;
		}

		void *id_expr = new(IdentifierExpr, id_token, id_symbol);

		if(P_MATCH(self, TOKEN_TYPE_LSQUARE_BRACE)) {

			const struct Symbol *id_sym = id_symbol;

			if(DEF_TYPE_VARIABLE != id_sym->definition_type && DEF_TYPE_CONSTANT != id_sym->definition_type) {
				p_raise_error(
					self, id_token,
					"Error: '%.*s' is not a variable.", id_token->length, id_token->start
				);

				return NULL;
			}

			const struct Token *lsbrace = CUR_TOKEN(self);
			p_advance(self);

			void *index_expr = expr(self);
			if(NULL == index_expr) {
				return NULL;
			}

			const struct Token *rsbrace = NULL;

			if(!P_MATCH(self, TOKEN_TYPE_RSQUARE_BRACE)) {
				p_raise_error(
					self, lsbrace,
					"SyntaxError: Unmatched '[', expected corressponding ']'."
				);
				return NULL;
			}
			else {
				rsbrace = CUR_TOKEN(self);
				p_advance(self);
			}

			return new(ListSubscriptExpr, id_expr, lsbrace, index_expr, rsbrace);
		}
		
		return id_expr;
	}

	else if(P_MATCH(self, TOKEN_TYPE_LPAREN)) {
		const struct Token *lparen = CUR_TOKEN(self);
		p_advance(self);

		void *child_expr = expr(self);

		if(NULL == child_expr)
			return NULL;

		const struct Token *rparen = NULL;

		if(!P_MATCH(self, TOKEN_TYPE_RPAREN)) {
			p_raise_error(
				self, lparen,
				"SyntaxError: Expected corresponding ')' for '('."
			);

			delete(child_expr);
			return NULL;
		}
		else {
			rparen = CUR_TOKEN(self);
			p_advance(self);
		}

		return new(GroupExpr, lparen, child_expr, rparen);
	}

	else if(P_MATCH(self, TOKEN_TYPE_LSQUARE_BRACE)) {
		const struct Token *lsbrace = CUR_TOKEN(self);
		p_advance(self);

		void *child_expr = expr(self);

		if(NULL == child_expr)
			return NULL;

		uint32_t expr_count = 1;
		void **expressions = allocate(sizeof(void *), expr_count);
		expressions[expr_count - 1] = child_expr;
		
		while(P_MATCH(self, TOKEN_TYPE_COMMA)) {
			const struct Token *comma_token = CUR_TOKEN(self);
			p_advance(self);

			child_expr = expr(self);

			if(NULL == child_expr)
				return NULL;
			
			expressions = reallocate(expressions, sizeof(void *), ++expr_count);
			expressions[expr_count - 1] = child_expr;
		}

		const struct Token *rsbrace = NULL;

		if(!P_MATCH(self, TOKEN_TYPE_RSQUARE_BRACE)) {
			p_raise_error(
				self, lsbrace,
				"SyntaxError: Unmatched '[', expected corresponding ']'."
			);

			for(uint32_t i = 0; i < expr_count; i++)
				delete(expressions[i]);

			free(expressions);

			return NULL;
		}
		else {
			rsbrace = CUR_TOKEN(self);
			p_advance(self);
		}

		return new(ListExpr, lsbrace, expr_count, expressions, rsbrace);
	}

	// const struct Token *cur = CUR_TOKEN(self);
	const struct Token *prev = PREV_TOKEN(self);

	p_raise_error(
		self, prev,
		"SyntaxError: Expected some expression after '%.*s'.", prev->length, prev->start
	);

	// if(cur->token_type.ordinal == TOKEN_TYPE_EOF.ordinal) {
	// 	p_raise_error(
	// 		self, PREV_TOKEN(self),
	// 		"Error: Unexpected EOF."
	// 	);
	// }
	// else {
	// 	p_raise_error(
	// 		self, cur,
	// 		// 	"SyntaxError: Expected some expression after '('."
	// 		"Error: Unexpected '%.*s'.", cur->length, cur->start
	// 	);
	// }

	return NULL;
}









void parse(void *_self) {
	// time_t start_time;
	// time(&start_time);

	struct Parser *self = _self;
	assert(IS_PARSER(self));

	self->symbol_table_stack = new(SymbolTableStack);

	void *predef_table = (void *) symtabstack_push(self->symbol_table_stack);		// symbol_table for predefined stuff

	// declare predefined stuff
	declare_native_funs(predef_table);

	self->program_ast = program(self);

	// time_t end_time;
	// time(&end_time);

	// printf("Total Parsing time: %gs\n", difftime(end_time, start_time));
}


#undef MAX_PARSE_ERRORS

#undef CUR_TOKEN
#undef PREV_TOKEN

#undef P_MATCH

