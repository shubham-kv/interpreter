#include "interpreter.h"

#include <assert.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include "type.h"
#include "object.h"
#include "main.h"
#include "value.h"
#include "value_object.h"
#include "value_list.h"
#include "memory.h"

#include "my_lexer.h"
#include "token_type.h"
#include "token.h"

#include "predefined.h"

#include "ast_node_headers.h"

#include "ast_visitor.h"
#include "ast_object.h"

#include "symbol.h"
#include "symbol_table.h"
#include "symbol_table_stack.h"

// #include "runtime_err_handler.h"

#include "runtime_stack.h"
#include "stack_frame.h"
#include "frame_pointer.h"

#include "values.h"



#define VISIT_NODE(ast_visitor, ast_node) (accept_visitor(ast_node, ast_visitor))


struct Interpreter {
	const void *class;

	const void *lexer;

	const void *program_ast;
	const void *symbol_table_stack;

	const void *ast_visitor;
	void *runtime_stack;

	void *ret_value;
	bool ret_flag;
	bool break_flag;
	bool continue_flag;
};


static void *interpreter_constructor(void *self, va_list *ap);
static void *interpreter_destructor(void *self);

static const struct Type _type_interpreter = {
	sizeof(struct Interpreter),
	interpreter_constructor,
	interpreter_destructor
};

const void *Interpreter = &_type_interpreter;



// important object pointer
static struct Interpreter *interpreter;


static void *i_visit_program_ast	(const void *ast_visitor, const void *program_ast);

static void *i_visit_var_decl		(const void *ast_visitor, const void *var_decl);
static void *i_visit_fun_decl		(const void *ast_visitor, const void *fun_decl);
static void *i_visit_class_decl		(const void *ast_visitor, const void *class_decl);

static void *i_execute_expr_stmt	(const void *ast_visitor, const void *expr_stmt);
static void *i_execute_print_stmt	(const void *ast_visitor, const void *print_stmt);

static void *i_execute_if_stmt		(const void *ast_visitor, const void *if_stmt);
static void *i_execute_when_stmt	(const void *ast_visitor, const void *when_stmt);

static void *i_execute_loop_stmt	(const void *ast_visitor, const void *loop_stmt);
static void *i_execute_repeat_stmt	(const void *ast_visitor, const void *repeat_stmt);

static void *i_execute_block_stmt	(const void *ast_visitor, const void *block_stmt);

static void *i_execute_break_stmt	(const void *ast_visitor, const void *break_stmt);
static void *i_execute_continue_stmt(const void *ast_visitor, const void *contnue_stmt);
static void *i_execute_ret_stmt		(const void *ast_visitor, const void *ret_stmt);


static void *i_evaluate_id_expr		(const void *ast_visitor, const void *id_expr);
static void *i_evaluate_literal_expr(const void *ast_visitor, const void *literal_expr);

static void *i_evaluate_unary_expr	(const void *ast_visitor, const void *unary_expr);
static void *i_evaluate_binary_expr	(const void *ast_visitor, const void *binary_expr);
static void *i_evaluate_logical_expr(const void *ast_visitor, const void *logical_expr);

static void *i_evaluate_group_expr	(const void *ast_visitor, const void *group_expr);

static void *i_evaluate_call_expr	(const void *ast_visitor, const void *call_expr);
static void *i_evaluate_get_expr	(const void *ast_visitor, const void *get_expr);

// static void *i_evaluate_assign_expr		(const void *ast_visitor, const void *assign_expr);
static void *i_evaluate_id_assign_expr	(const void *ast_visitor, const void *id_assign_expr);
static void *i_evaluate_ls_assign_expr	(const void *ast_visitor, const void *ls_assign_expr);
static void *i_evaluate_set_expr		(const void *ast_visitor, const void *set_expr);


static void *i_evaluate_list_expr	(const void *ast_visitor, const void *list_expr);

static void *i_evaluate_list_subscript_expr(const void *ast_visitor, const void *ls_expr);


// ... = new(Interpreter, lexer, program_ast, runtime_stack);

static void *interpreter_constructor(void *_self, va_list *ap) {
	struct Interpreter *self = _self;
	assert(self && (self->class == Interpreter));

	self->lexer = va_arg(*ap, const void *);
	assert(IS_LEXER(self->lexer));

	self->program_ast = va_arg(*ap, const void *);
	assert(IS_PROGRAM_AST(self->program_ast));

	self->symbol_table_stack = va_arg(*ap, const void *);
	assert(IS_SYMBOL_TABLE_STACK(self->symbol_table_stack));

	self->ast_visitor = new(
		AstVisitor,
		i_visit_program_ast,
		
		i_visit_var_decl,
		i_visit_fun_decl,
		i_visit_class_decl,
		
		i_execute_expr_stmt,
		i_execute_print_stmt,
		
		i_execute_if_stmt,
		i_execute_when_stmt,
		
		i_execute_loop_stmt,
		i_execute_repeat_stmt,
		
		i_execute_block_stmt,

		i_execute_break_stmt,
		i_execute_continue_stmt,
		i_execute_ret_stmt,

		i_evaluate_id_expr,
		i_evaluate_literal_expr,

		i_evaluate_unary_expr,
		i_evaluate_binary_expr,

		i_evaluate_logical_expr,
		i_evaluate_group_expr,

		i_evaluate_call_expr,
		i_evaluate_get_expr,

		i_evaluate_id_assign_expr,
		i_evaluate_ls_assign_expr,
		i_evaluate_set_expr,

		i_evaluate_list_expr,

		i_evaluate_list_subscript_expr
	);

	self->runtime_stack = new(RuntimeStack);

	self->ret_flag = false;
	self->ret_value = NULL;

	self->break_flag = false;
	self->continue_flag = false;

	interpreter = self;

	return _self;
}

static void *interpreter_destructor(void *_self) {
	struct Interpreter *self = _self;
	assert(self && (self->class == Interpreter));

	delete((void *) self->ast_visitor);
	delete((void *) self->runtime_stack);

	if(self == interpreter)
		interpreter = NULL;

	return _self;
}


void i_report_error_va(
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
		
		fprintf(stderr, "\n");
	}

	exit(EXIT_FAILURE);
}


void i_raise_error(
	const struct Token *err_token,
	const char *err_msg, ...
) {

	va_list ap;
	va_start(ap, err_msg);

	if(NULL == err_token) {
		i_report_error_va(false, 0, NULL, 0, 0, err_msg, &ap);
	}
	else {
		const int line_index = err_token->line_index;
		const char *err_line = LEXER_GET_LINE(interpreter->lexer, line_index);
		const int err_index = err_token->start_index;

		i_report_error_va(
			true, (line_index + 1),
			err_line, err_index, err_token->length, err_msg, &ap
		);
	}

	va_end(ap);
}



void i_raise_error_expr(
	struct Interpreter *self,
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

	exit(EXIT_FAILURE);
}











static void *i_visit_program_ast(const void *ast_visitor, const void *_program_ast) {
	const struct ProgramAst *program_ast = _program_ast;
	assert(IS_PROGRAM_AST(program_ast));

	rstack_push(interpreter->runtime_stack, program_ast->symbol_table);	// program's stack frame

	for(uint32_t i = 0; NULL != program_ast->declarations[i]; i++)
		VISIT_NODE(ast_visitor, program_ast->declarations[i]);

	// TODO: try printing the frame's contents before clean up
	print_stack_frame(rstack_get_local_frame(interpreter->runtime_stack));

	rstack_pop(interpreter->runtime_stack);	// program's stack frame

	return NULL;
}






/*
|==============================>
| Declaration visitor methods.
|==============================>
*/

static void *i_visit_var_decl(const void *ast_visitor, const void *_var_decl) {
	const struct VarDecl *var_decl = _var_decl;
	assert(IS_VAR_DECL(var_decl));

	const void *local_frame = rstack_get_local_frame(interpreter->runtime_stack);
	const void *symbol = NULL;
	const void *init_expr = NULL;

	for(uint32_t i = 0; NULL != var_decl->symbols[i]; i++) {
		symbol = var_decl->symbols[i];
		init_expr = symbol_get_attr(symbol, SYM_ATTR_KEY_INIT_EXPR);

		if(NULL != init_expr) {
			const void *value = VISIT_NODE(ast_visitor, init_expr);
			sframe_set_value((void *) local_frame, SYMBOL_GET_ID(symbol), value);
		}
	}

	return NULL;
}



static void *execute_fun(const void *_fun, void **args) {
	const struct FunValue *fun = _fun;
	assert(IS_FUN_VALUE(fun));

	for(uint32_t i = 0; fun->declarations[i] && !interpreter->ret_flag; i++) {
		VISIT_NODE(interpreter->ast_visitor, fun->declarations[i]);
	}

	return interpreter->ret_value;
}


static void *i_visit_fun_decl(
	const void *ast_visitor, const void *_fun_decl
) {

	const struct FunDecl *fun_decl = _fun_decl;
	assert(IS_FUN_DECL(fun_decl));

	const void *local_frame = rstack_get_local_frame(interpreter->runtime_stack);
	const void *fun_symbol = FUN_DECL_GET_SYMBOL(fun_decl);

	const void *fun_value = sframe_get_value(local_frame, SYMBOL_GET_ID(fun_symbol));

	if(NULL == fun_value) {
		fun_value =
			new(
				FunValue, fun_symbol,
				fun_decl->symbol_table,
				fun_decl->param_count,
				fun_decl->param_symbols,
				execute_fun,
				fun_decl->declarations
			);

		sframe_put_value((void *) local_frame, SYMBOL_GET_ID(fun_symbol), fun_value);
	}

	return NULL;
}




static void *constructor(const void *_class_value, void **args) {
	const struct ClassValue *class_value = _class_value;
	assert(IS_CLASS_VALUE(class_value));

	void *properties = new(HashTable);

	for(int i = 0; i < class_value->property_count; i++) {
		void *property_value = args[i];
		hashtable_put(properties, SYMBOL_GET_ID(class_value->properties[i]), property_value);
	}

	return new(InstanceValue, class_value, properties);
}


static void *i_visit_class_decl(
	const void *ast_visitor, const void *_class_decl
) {

	const struct ClassDecl *class_decl = _class_decl;
	assert(IS_CLASS_DECL(class_decl));

	const void *local_frame = rstack_get_local_frame(interpreter->runtime_stack);
	const void *class_symbol = class_decl->symbol;

	const void *class_value = sframe_get_value(local_frame, SYMBOL_GET_ID(class_symbol));

	if(NULL == class_value) {
		class_value = 
			new(ClassValue,
				class_decl->symbol, class_decl->symbol_table,
				class_decl->property_count, class_decl->properties,
				class_decl->methods,
				constructor
			);

		sframe_put_value((void *) local_frame, SYMBOL_GET_ID(class_symbol), class_value);
	}


	return NULL;
}



/*
|==============================>
| Statement executor methods.
|==============================>
*/

static void *i_execute_expr_stmt(const void *ast_visitor, const void *_expr_stmt) {
	const struct ExprStmt *expr_stmt = _expr_stmt;
	assert(IS_EXPR_STMT(expr_stmt));

	void *value = VISIT_NODE(ast_visitor, expr_stmt->expr);

	if(NULL != value)
		delete(value);
	
	return NULL;
}



static void *i_execute_print_stmt(const void *ast_visitor, const void *_print_stmt) {
// 	const struct PrintStmt *print_stmt = _print_stmt;
// 	assert(print_stmt->class == PrintStmt);
// 	void *expr_value = (void *) VISIT_NODE(ast_visitor, print_stmt->expr);
// 	v_print_value(expr_value);
// 	delete(expr_value);
	return NULL;
}



static void *i_execute_if_stmt(const void *ast_visitor, const void *_if_stmt) {
	const struct IfStmt *if_stmt = _if_stmt;
	assert(IS_IF_STMT(if_stmt));

	void *expr_value = VISIT_NODE(ast_visitor, if_stmt->expr);

	if(is_value_true(expr_value)) {
		VISIT_NODE(ast_visitor, if_stmt->if_block);
	} else if(NULL != if_stmt->else_block) {
		VISIT_NODE(ast_visitor, if_stmt->else_block);
	}

	delete(expr_value);
	return NULL;
}



static void *i_execute_when_stmt(const void *ast_visitor, const void *_when_stmt) {
	const struct WhenStmt *when_stmt = _when_stmt;
	assert(IS_WHEN_STMT(when_stmt));

	void *condition_expr_value = (void *) VISIT_NODE(ast_visitor, when_stmt->expr);

	const void *const *when_case_exprs = NULL;
	void *when_case_expr_value = NULL;

	bool is_done = false;

	for(uint32_t i = 0; (NULL != when_stmt->cases[i]) && (!is_done); i++) {
		when_case_exprs = (const void *const *) when_stmt->cases[i]->exprs;

		for(uint32_t j = 0; NULL != when_case_exprs[j]; j++) {
			when_case_expr_value = (void *) VISIT_NODE(ast_visitor, when_case_exprs[j]);

			if(are_values_equal(condition_expr_value, when_case_expr_value)) {
				delete(when_case_expr_value);

				VISIT_NODE(ast_visitor, when_stmt->cases[i]->stmt);

				is_done = true;
				break;
			}

			delete(when_case_expr_value);
			when_case_expr_value = NULL;
		}
	}

	if(!is_done) {
		if(NULL != when_stmt->else_stmt)
			VISIT_NODE(ast_visitor, when_stmt->else_stmt);
	}

	delete(condition_expr_value);

	return NULL;
}



static void *i_execute_loop_stmt(const void *ast_visitor, const void *_loop_stmt) {
	const struct LoopStmt *loop_stmt = _loop_stmt;
	assert(IS_LOOP_STMT(loop_stmt));

	void *start_expr_val = NULL;

	rstack_push(interpreter->runtime_stack, loop_stmt->symbol_table);	// loop's stack frame

	if(loop_stmt->loop_type == LOOP_STMT_TYPE_CONDITIONAL) {
		while(1 && !interpreter->ret_flag && !interpreter->break_flag) {
			start_expr_val = VISIT_NODE(ast_visitor, loop_stmt->start_expr);

			if(is_value_true(start_expr_val)) {
				delete(start_expr_val);

				for(uint32_t i = 0; NULL != loop_stmt->child_stmts[i] && !interpreter->ret_flag && !interpreter->break_flag; i++) {
					if(interpreter->continue_flag) {
						interpreter->continue_flag = false;
						break;
					}

					VISIT_NODE(ast_visitor, loop_stmt->child_stmts[i]);
				}
			}
			else {
				break;
			}
		}
		delete(start_expr_val);
		rstack_pop(interpreter->runtime_stack);		// loop's stack frame

		interpreter->break_flag = false;

		return NULL;
	}

	// else
	start_expr_val = VISIT_NODE(ast_visitor, loop_stmt->start_expr);



	char *loop_it_id = NULL;

	if(NULL != loop_stmt->id_token) {
		const struct Token *id_token = loop_stmt->id_token;

		loop_it_id = allocate(sizeof(char), id_token->length + 1);
		strncpy(loop_it_id, id_token->start, id_token->length);
	}
	else {
		const char *loop_iterator = "it";

		loop_it_id = allocate(sizeof(char), strlen(loop_iterator) + 1);
		strncpy(loop_it_id, loop_iterator, strlen(loop_iterator));
	}


	// const void *loop_it_symbol = symbol_table_lookup(loop_stmt->symbol_table, "it");
	const void *loop_it_symbol = symbol_table_lookup(loop_stmt->symbol_table, loop_it_id);
	free(loop_it_id);

	void *end_expr_val = VISIT_NODE(ast_visitor, loop_stmt->end_expr);
	void *step_expr_val = NULL;

	int start_val = VALUE_AS_INT(start_expr_val);
	int end_val = VALUE_AS_INT(end_expr_val);
	int step_val = 1;

	if(NULL != loop_stmt->step_expr) {
		step_expr_val = VISIT_NODE(ast_visitor, loop_stmt->step_expr);
		step_val = VALUE_AS_INT(step_expr_val);
	}

	if(start_val < end_val) {
		int upper_limit = (loop_stmt->arrow_token->token_type.ordinal == TOKEN_TYPE_EQUAL_ARROW.ordinal) ? (end_val + 1) : (end_val);

		if(step_val < 0) {
			i_raise_error(NULL, "RuntimeError: Step value can't be negative.");
		}

		for(int i = start_val; (i < upper_limit) && !interpreter->ret_flag && !interpreter->break_flag; i += step_val) {
			rstack_set_value(interpreter->runtime_stack, loop_it_symbol, new(Value, VALUE_TYPE_INTEGER, i));

			for(uint32_t i = 0; loop_stmt->child_stmts[i] && !interpreter->ret_flag && !interpreter->break_flag; i++) {
				if(interpreter->continue_flag) {
					interpreter->continue_flag = false;
					break;
				}
				VISIT_NODE(ast_visitor, loop_stmt->child_stmts[i]);
			}
		}
	}
	else if(end_val < start_val) {  }
	else {
		if(loop_stmt->arrow_token->token_type.ordinal == TOKEN_TYPE_EQUAL_ARROW.ordinal) {
			rstack_set_value(interpreter->runtime_stack, loop_it_symbol, new(Value, VALUE_TYPE_INTEGER, start_val));

			for(uint32_t i = 0; loop_stmt->child_stmts[i] && !interpreter->ret_flag && !interpreter->break_flag; i++) {
				if(interpreter->continue_flag) {
					interpreter->continue_flag = false;
					break;
				}
				VISIT_NODE(ast_visitor, loop_stmt->child_stmts[i]);
			}
		}
	}

	delete(start_expr_val);
	delete(end_expr_val);

	if(NULL != step_expr_val)
		delete(step_expr_val);
	
	rstack_pop(interpreter->runtime_stack);		// loop's stack frame

	interpreter->break_flag = false;

	return NULL;
}



static void *i_execute_repeat_stmt(const void *ast_visitor, const void *_repeat_stmt) {
	const struct RepeatStmt *repeat_stmt = _repeat_stmt;
	assert(IS_REPEAT_STMT(repeat_stmt));

	rstack_push(interpreter->runtime_stack, repeat_stmt->symbol_table);		// repeat's stack_frame


	char *repeat_iterator_id = NULL;

	if(NULL != repeat_stmt->id_token) {
		const struct Token *id_token = repeat_stmt->id_token;

		repeat_iterator_id = allocate(sizeof(char), id_token->length + 1);
		strncpy(repeat_iterator_id, id_token->start, id_token->length);
	}
	else {
		const char *it = "it";

		repeat_iterator_id = allocate(sizeof(char), strlen(it) + 1);
		strncpy(repeat_iterator_id, it, strlen(it));
	}

	// const void *repeat_it_symbol = symbol_table_lookup(repeat_stmt->symbol_table, "it");
	const void *repeat_it_symbol = symbol_table_lookup(repeat_stmt->symbol_table, repeat_iterator_id);
	free(repeat_iterator_id);

	struct Value *expr_value = (struct Value *) VISIT_NODE(ast_visitor, repeat_stmt->expr);

	if(VALUE_TYPE_INTEGER == expr_value->value_type) {
		int ival = VALUE_AS_INT(expr_value);
		ival = (ival > 0) ? ival : (-1 * ival);

		for(uint32_t i = 0; i < ival && !interpreter->ret_flag && !interpreter->break_flag; i++) {
			rstack_set_value(interpreter->runtime_stack, repeat_it_symbol, new(Value, VALUE_TYPE_INTEGER, i));

			for(uint32_t j = 0; repeat_stmt->child_stmts[j] && !interpreter->ret_flag && !interpreter->break_flag; j++) {
				if(interpreter->continue_flag) {
					interpreter->continue_flag = false;
					break;
				}
				VISIT_NODE(ast_visitor, repeat_stmt->child_stmts[j]);
			}
		}
	}
	else {
		i_raise_error(NULL, "RuntimeError: Unexpected value type within repeat's parenthesis.");
	}

	delete(expr_value);

	rstack_pop(interpreter->runtime_stack);		// repeat's stack_frame

	interpreter->break_flag = false;

	return NULL;
}



static void *i_execute_block_stmt(const void *ast_visitor, const void *_block_stmt) {
	const struct BlockStmt *block_stmt = _block_stmt;
	assert(IS_BLOCK_STMT(block_stmt));

	rstack_push(interpreter->runtime_stack, block_stmt->symbol_table);

	for(uint32_t i = 0; block_stmt->child_stmts[i] && !interpreter->ret_flag; i++) {
		VISIT_NODE(ast_visitor, block_stmt->child_stmts[i]);
	}
	
	rstack_pop(interpreter->runtime_stack);		// block_stmt->symbol_table

	return NULL;
}



static void *i_execute_break_stmt(const void *ast_visitor, const void *_break_stmt) {
	const struct BreakStmt *break_stmt = _break_stmt;
	assert(IS_BREAK_STMT(break_stmt));

	interpreter->break_flag = true;
	return NULL;
}

static void *i_execute_continue_stmt(const void *ast_visitor, const void *_contnue_stmt) {
	const struct ContinueStmt *continue_stmt = _contnue_stmt;
	assert(IS_CONTINUE_STMT(continue_stmt));

	interpreter->continue_flag = true;
	return NULL;
}



static void *i_execute_ret_stmt(const void *ast_visitor, const void *_ret_stmt) {
	const struct RetStmt *ret_stmt = _ret_stmt;
	assert(IS_RET_STMT(ret_stmt));

	interpreter->ret_flag = true;

	if(NULL != ret_stmt->expression) {
		interpreter->ret_value = VISIT_NODE(ast_visitor, ret_stmt->expression);
	} else {
		interpreter->ret_value = NULL;
	}
	
	return NULL;
}



/*
|==============================>
| Expression ast_visitor methods.
|==============================>
*/

static void *i_evaluate_id_expr(const void *ast_visitor, const void *_id_expr) {
	const struct IdentifierExpr *id_expr = _id_expr;
	assert(IS_IDENTIFIER_EXPR(id_expr));

	const void *value = rstack_get_value(interpreter->runtime_stack, id_expr->symbol);
	return copy_value(value);
}



static void *i_evaluate_literal_expr(const void *ast_visitor, const void *_literal_expr) {
	const struct LiteralExpr *literal_expr = _literal_expr;
	assert(IS_LITERAL_EXPR(literal_expr));

	const uint32_t ordinal = literal_expr->literal_token->token_type.ordinal;

	if(ordinal == TOKEN_TYPE_INTEGER.ordinal) {
		int ival = strtol(literal_expr->literal_token->start, NULL, 10);

		return new(Value, VALUE_TYPE_INTEGER, ival);
	}

	else if(ordinal == TOKEN_TYPE_DOUBLE.ordinal) {
		double dval = strtod(literal_expr->literal_token->start, NULL);

		return new(Value, VALUE_TYPE_DOUBLE, dval);
	}

	else if(ordinal == TOKEN_TYPE_STRING.ordinal) {
		// - 2 for the two enclosing quotes
		size_t str_size = literal_expr->literal_token->length - 2;

		void *strval = calloc(str_size + 1, sizeof(char));
		strncpy(strval, literal_expr->literal_token->start + 1, str_size);

		return new(Value, VALUE_TYPE_STRING, strval);
	}

	else if(ordinal == TOKEN_TYPE_TRUE.ordinal) {
		return new(Value, VALUE_TYPE_BOOL, 1);
	}

	else if(ordinal == TOKEN_TYPE_FALSE.ordinal) {
		return new(Value, VALUE_TYPE_BOOL, 0);
	}

	return NULL;
}



static void *i_evaluate_unary_expr(const void *ast_visitor, const void *_unary_expr) {
	const struct UnaryExpr *unary_expr = _unary_expr;
	assert(IS_UNARY_EXPR(unary_expr));

	struct Value *right_expr_value = VISIT_NODE(ast_visitor, unary_expr->right);
	assert(IS_VALUE(right_expr_value));

	const uint8_t ordinal = unary_expr->op_token->token_type.ordinal;

	if(ordinal == TOKEN_TYPE_PLUS.ordinal) {
		return right_expr_value;
	}

	else if(ordinal == TOKEN_TYPE_MINUS.ordinal) {
		switch(right_expr_value->value_type) {
			case VALUE_TYPE_INTEGER: {
				int ival = VALUE_AS_INT(right_expr_value);
				delete(right_expr_value);

				return new(Value, VALUE_TYPE_INTEGER, (-1 * ival));
			}

			case VALUE_TYPE_DOUBLE: {
				double dval = VALUE_AS_DOUBLE(right_expr_value);
				delete(right_expr_value);

				return new(Value, VALUE_TYPE_DOUBLE, (-1 * dval));
			}
			default: {
				i_raise_error_expr(
					interpreter, unary_expr,
					"RuntimeError: Incompatible operand for operator '%.*s'.",
					unary_expr->op_token->length, unary_expr->op_token->start
				);
				return NULL;
			}
		}
	}

	else if (ordinal == TOKEN_TYPE_NOT.ordinal) {
		void *res_value = new(Value, VALUE_TYPE_BOOL, !is_value_true(right_expr_value));
		delete(right_expr_value);

		return res_value;
	}

	// should be unreachable
	return NULL;
}



static void *i_evaluate_binary_expr(const void *ast_visitor, const void *_binary_expr) {
	const struct BinaryExpr *binary_expr = _binary_expr;
	assert(IS_BINARY_EXPR(binary_expr));

	struct Value *left_value = (struct Value *) VISIT_NODE(ast_visitor, binary_expr->left);
	struct Value *right_value = (struct Value *) VISIT_NODE(ast_visitor, binary_expr->right);

	const enum ValueType left_value_type = left_value->value_type;
	const enum ValueType right_value_type = right_value->value_type;


	#define BINARY_OP_COMPARATIVE(op_token, operator, left_value, right_value) \
		do { \
			bool res; \
			bool res_value_evaluated = false; \
			\
			if(left_value_type == right_value_type) { \
				\
				if(VALUE_TYPE_INTEGER == left_value_type) { \
					int val1 = left_value->value.integer_val; \
					int val2 = right_value->value.integer_val;\
					res = (val1 operator val2); \
					res_value_evaluated = true; \
				} \
				else if(VALUE_TYPE_DOUBLE == left_value_type) { \
					double val1 = left_value->value.double_val; \
					double val2 = right_value->value.double_val;\
					res = (val1 operator val2); \
					res_value_evaluated = true; \
				} \
				else if(VALUE_TYPE_BOOL == left_value_type) { \
					bool val1 = left_value->value.integer_val; \
					bool val2 = right_value->value.integer_val;\
					res = (val1 operator val2); \
					res_value_evaluated = true; \
				} \
				delete((void *) left_value);  \
				delete((void *) right_value); \
				\
				if(res_value_evaluated) \
					return new(Value, VALUE_TYPE_BOOL, res); \
				\
				i_raise_error( \
					op_token, \
					"RuntimeError: Incompatible operands for operator '%.*s'.", \
					op_token->length, op_token->start \
				); \
			} \
			\
			if((VALUE_TYPE_INTEGER == left_value_type) && (VALUE_TYPE_DOUBLE == right_value_type)) { \
				int val1 = left_value->value.integer_val; \
				double val2 = right_value->value.double_val; \
				res = (val1 operator val2); \
				res_value_evaluated = true; \
			} \
			else if((VALUE_TYPE_DOUBLE == left_value_type) && (VALUE_TYPE_INTEGER == right_value_type)) { \
				double val1 = left_value->value.double_val; \
				int val2 = right_value->value.integer_val; \
				res = (val1 operator val2); \
				res_value_evaluated = true; \
			} \
			delete((void *) left_value); \
			delete((void *) right_value); \
			\
			if(res_value_evaluated) \
				return new(Value, VALUE_TYPE_BOOL, res); \
			\
			i_raise_error( \
				op_token, \
				"RuntimeError: Incompatible operands for operator '%.*s'.", \
				op_token->length, op_token->start \
			); \
		} while(0)


	const uint8_t ordinal = binary_expr->op_token->token_type.ordinal;

	void *res = NULL;

	if(ordinal == TOKEN_TYPE_EQUAL_EQUAL.ordinal)
		BINARY_OP_COMPARATIVE(binary_expr->op_token, ==, left_value, right_value);

	else if(ordinal == TOKEN_TYPE_NOT_EQUAL.ordinal)
		BINARY_OP_COMPARATIVE(binary_expr->op_token, !=, left_value, right_value);

	else if(ordinal == TOKEN_TYPE_LESS_THAN.ordinal)
		BINARY_OP_COMPARATIVE(binary_expr->op_token, <, left_value, right_value);

	else if(ordinal == TOKEN_TYPE_LESS_THAN_EQUALS.ordinal)
		BINARY_OP_COMPARATIVE(binary_expr->op_token, <=, left_value, right_value);

	else if(ordinal == TOKEN_TYPE_GREATER_THAN.ordinal)
		BINARY_OP_COMPARATIVE(binary_expr->op_token, >, left_value, right_value);
	
	else if(ordinal == TOKEN_TYPE_GREATER_THAN_EQUALS.ordinal)
		BINARY_OP_COMPARATIVE(binary_expr->op_token, >=, left_value, right_value);

	else if(ordinal == TOKEN_TYPE_PLUS.ordinal)
		res = add_values(left_value, right_value);

	else if(ordinal == TOKEN_TYPE_MINUS.ordinal)
		res = sub_values(left_value, right_value);

	else if(ordinal == TOKEN_TYPE_STAR.ordinal)
		res = mul_values(left_value, right_value);

	else if((ordinal == TOKEN_TYPE_FORWARD_SLASH.ordinal) || (ordinal == TOKEN_TYPE_PERCENT.ordinal)) {

		if((VALUE_TYPE_INTEGER == left_value_type) || (VALUE_TYPE_DOUBLE == left_value_type)) {
			if(VALUE_TYPE_INTEGER == right_value_type) {
				if(0 == VALUE_AS_INT(right_value)) {
					i_raise_error_expr(
						interpreter, binary_expr,
						"RuntimeError: Division by zero."
					);
					return NULL;
				}
			}
			else if(VALUE_TYPE_DOUBLE == right_value_type) {
				if(0.0 == VALUE_AS_DOUBLE(right_value)) {
					i_raise_error_expr(
						interpreter, binary_expr,
						"RuntimeError: Division by zero."
					);
					return NULL;
				}
			}
		}

		if(ordinal == TOKEN_TYPE_FORWARD_SLASH.ordinal) {
			res = div_values(left_value, right_value);
		} else {
			res = mod_values(left_value, right_value);
		}
	}

	delete(left_value);
	delete(right_value);

	if(NULL != res)
		return res;

	i_raise_error_expr(
		interpreter, binary_expr,
		"RuntimeError: Incompatible operands for operator '%.*s'.",
		binary_expr->op_token->length, binary_expr->op_token->start
	);
	
	#undef BINARY_OP_COMPARATIVE

	// must be unreachable
	return NULL;
}



static void *i_evaluate_logical_expr(const void *ast_visitor, const void *_logical_expr) {
	const struct LogicalExpr *logical_expr = _logical_expr;
	assert(IS_LOGICAL_EXPR(logical_expr));

	void *left_value = VISIT_NODE(ast_visitor, logical_expr->left_expr);
	void *res_value = NULL;

	const uint32_t ordinal = logical_expr->op_token->token_type.ordinal;

	if(ordinal == TOKEN_TYPE_LOGICAL_OR.ordinal) {
		if(is_value_true(left_value)) {
			res_value = new(Value, VALUE_TYPE_BOOL, true);
		}
		else {
			void *right_value = VISIT_NODE(ast_visitor, logical_expr->right_expr);
			res_value = new(Value, VALUE_TYPE_BOOL, is_value_true(right_value));
			delete(right_value);
		}
	}
	else if(ordinal == TOKEN_TYPE_LOGICAL_AND.ordinal) {
		if(!is_value_true(left_value)) {
			res_value = new(Value, VALUE_TYPE_BOOL, false);
		}
		else {
			void *right_value = VISIT_NODE(ast_visitor, logical_expr->right_expr);
			res_value = new(Value, VALUE_TYPE_BOOL, is_value_true(right_value));
			delete(right_value);
		}
	}

	delete(left_value);

	return res_value;
}



static void *i_evaluate_group_expr(const void *ast_visitor, const void *_group_expr) {
	const struct GroupExpr *group_expr = _group_expr;
	assert(IS_GROUP_EXPR(group_expr));

	return VISIT_NODE(ast_visitor, group_expr->child_expr);
}


/* static void *i_evaluate_call_expr(const void *ast_visitor, const void *_call_expr) {
	const struct CallExpr *call_expr = _call_expr;
	assert(IS_CALL_EXPR(call_expr));

	void *value = VISIT_NODE(interpreter->ast_visitor, call_expr->left_expr);

	if((NULL == value) || !IS_FUN_VALUE(value)) {
		i_raise_error(NULL, "Value is not callable.");
	}

	struct FunValue *fun = value;
	assert(IS_FUN_VALUE(fun));

	if(call_expr->arg_count < fun->param_count) {
		i_raise_error(
			NULL, "Error: Too few arguments passed to '%s', expected %d argument(s), passed %d.",
			SYMBOL_GET_ID(fun->symbol),
			fun->param_count, call_expr->arg_count
		);
		return NULL;
	}
	else if(call_expr->arg_count > fun->param_count) {
		i_raise_error(
			NULL, "Error: Too many arguments passed to '%s', expected %d argument(s), passed %d.",
			SYMBOL_GET_ID(fun->symbol),
			fun->param_count, call_expr->arg_count
		);

		// p_raise_error(
		// 	self, id_token,
		// 	"Error: Too many arguments passed to '%.*s', expected %d argument(s), passed %d.",
		// 	id_token->length, id_token->start, param_count, arg_count
		// );

		return NULL;
	}

	void **arguments = allocate(sizeof(void *), fun->param_count);

	for(uint32_t i = 0; i < fun->param_count; i++) {
		const void *arg_expr = call_expr->args[i];
		arguments[i] = VISIT_NODE(ast_visitor, arg_expr);
	}

	// void *ret_value;
	// if(DEF_TYPE_NATIVE_FUN == SYMBOL_GET_DEF_TYPE(fun->symbol)) {
		void *fun_stack_frame = (void *) rstack_push(interpreter->runtime_stack, fun->symbol_table);

		for(uint32_t i = 0; i < fun->param_count; i++) {
			const struct Symbol *param = fun->param_symbols[i];
			sframe_set_value(fun_stack_frame, param->id, arguments[i]);
		}
	// }

	void *ret_value = fun->execute(fun, arguments);

	// if(DEF_TYPE_NATIVE_FUN == SYMBOL_GET_DEF_TYPE(fun->symbol)) {
		rstack_pop(interpreter->runtime_stack);
	// }

	free(arguments);

	// cuz evaluating id_expr already returns a copy, so when se're done delete the copy
	delete(fun);

	interpreter->ret_flag = false;
	interpreter->ret_value = NULL;

	return ret_value;



	// const struct Symbol *fun_symbol = call_expr->fun_symbol;
	// void *ret_value = NULL;
	// if(DEF_TYPE_NATIVE_FUN == fun_symbol->definition_type) {
	// 	const void *_function = symbol_get_attr(call_expr->fun_symbol, SYM_ATTR_KEY_FUN);
	// 	const struct FunValue *fun = _function;
	// 	assert(IS_FUN_VALUE(fun));
	// 	// evaluate the arg expr
		// void **arg_values = allocate(sizeof(void *), fun->param_count);
		// for(uint32_t i = 0; i < fun->param_count; i++) {
		// 	const void *arg_expr = call_expr->args[i];
		// 	arg_values[i] = VISIT_NODE(ast_visitor, arg_expr);
		// }
	// 	void *fun_stack_frame = (void *) rstack_push(interpreter->runtime_stack, fun->symbol_table);
	// 	for(uint32_t i = 0; i < fun->param_count; i++) {
	// 		const struct Symbol *param = fun->param_symbols[i];
	// 		sframe_set_value(fun_stack_frame, param->id, arg_values[i]);
	// 	}
	// 	ret_value = fun->execute(fun, arg_values);
	// 	rstack_pop(interpreter->runtime_stack);
	// 	free(arg_values);
	// 	interpreter->ret_flag = false;
	// 	interpreter->ret_value = NULL;
	// }
	// else if(DEF_TYPE_FUN == fun_symbol->definition_type) {
	// 	const void *_function = symbol_get_attr(call_expr->fun_symbol, SYM_ATTR_KEY_AST);
	// 	const struct FunDecl *fun = _function;
	// 	assert(IS_FUN_DECL(fun));
	// 	// evaluate the arg expr
	// 	void **arg_values = allocate(sizeof(void *), fun->param_count);
	// 	for(uint32_t i = 0; i < fun->param_count; i++) {
	// 		const void *arg_expr = call_expr->args[i];
	// 		arg_values[i] = VISIT_NODE(ast_visitor, arg_expr);
	// 	}
	// 	void *fun_stack_frame = (void *) rstack_push(interpreter->runtime_stack, fun->symbol_table);
	// 	for(uint32_t i = 0; i < fun->param_count; i++) {
	// 		const struct Symbol *param = fun->param_symbols[i];
	// 		sframe_set_value(fun_stack_frame, param->id, arg_values[i]);
	// 	}
	// 	for(uint32_t i = 0; fun->declarations[i] && !interpreter->ret_flag; i++) {
	// 		VISIT_NODE(ast_visitor, fun->declarations[i]);
	// 	}
	// 	ret_value = interpreter->ret_value;
	// 	rstack_pop(interpreter->runtime_stack);
	// 	free(arg_values);
	// 	interpreter->ret_flag = false;
	// 	interpreter->ret_value = NULL;
	// }
	// return ret_value;
}
*/


static void *i_evaluate_call_expr(const void *ast_visitor, const void *_call_expr) {
	const struct CallExpr *call_expr = _call_expr;
	assert(IS_CALL_EXPR(call_expr));


	void *value = VISIT_NODE(interpreter->ast_visitor, call_expr->left_expr);

	if((NULL == value) && !IS_FUN_VALUE(value) && !IS_CLASS_VALUE(value)) {
		i_raise_error(NULL, "Value is not callable.");
	}


	if(IS_FUN_VALUE(value)) {
		struct FunValue *fun = value;
		assert(IS_FUN_VALUE(fun));

		if(call_expr->arg_count < fun->param_count) {
			i_raise_error(
				NULL, "Error: Too few arguments passed to '%s', expected %d argument(s), passed %d.",
				SYMBOL_GET_ID(fun->symbol),
				fun->param_count, call_expr->arg_count
			);
			return NULL;
		}
		else if(call_expr->arg_count > fun->param_count) {
			i_raise_error(
				NULL, "Error: Too many arguments passed to '%s', expected %d argument(s), passed %d.",
				SYMBOL_GET_ID(fun->symbol),
				fun->param_count, call_expr->arg_count
			);
			return NULL;
		}

		void **arguments = allocate(sizeof(void *), fun->param_count);

		for(uint32_t i = 0; i < fun->param_count; i++) {
			const void *arg_expr = call_expr->args[i];
			arguments[i] = VISIT_NODE(ast_visitor, arg_expr);
		}

		void *fun_stack_frame = (void *) rstack_push(interpreter->runtime_stack, fun->symbol_table);

		for(uint32_t i = 0; i < fun->param_count; i++) {
			const struct Symbol *param = fun->param_symbols[i];
			sframe_set_value(fun_stack_frame, param->id, arguments[i]);
		}

		void *ret_value = fun->execute(fun, arguments);

		rstack_pop(interpreter->runtime_stack);

		free(arguments);

		delete(fun);

		interpreter->ret_flag = false;
		interpreter->ret_value = NULL;

		return ret_value;
	}
	else if(IS_CLASS_VALUE(value)) {

		const struct ClassValue *klass = value;

		if(call_expr->arg_count < klass->property_count) {
			i_raise_error(
				NULL, "Error: Too few arguments passed to '%s', expected %d argument(s), passed %d.",
				SYMBOL_GET_ID(klass->symbol),
				klass->property_count, call_expr->arg_count
			);
			return NULL;
		}
		else if(call_expr->arg_count > klass->property_count) {
			i_raise_error(
				NULL, "Error: Too many arguments passed to '%s', expected %d argument(s), passed %d.",
				SYMBOL_GET_ID(klass->symbol),
				klass->property_count, call_expr->arg_count
			);
			return NULL;
		}

		void **arguments = allocate(sizeof(void *), klass->property_count);

		for(uint32_t i = 0; i < klass->property_count; i++) {
			const void *arg_expr = call_expr->args[i];
			arguments[i] = VISIT_NODE(ast_visitor, arg_expr);
		}

		void *ret_value = klass->constructor(klass, arguments);
		free(arguments);
		delete((void *) klass);

		return ret_value;
	}

	// shouldn't reach here
	return NULL;
}






static void *i_evaluate_get_expr(const void *ast_visitor, const void *_get_expr) {
	const struct GetExpr *get_expr = _get_expr;
	assert(IS_GET_EXPR(get_expr));

	const void *_instance_value = VISIT_NODE(interpreter->ast_visitor, get_expr->left_expr);

	if(!IS_INSTANCE_VALUE(_instance_value)) {
		i_raise_error_expr(
			interpreter, get_expr->left_expr,
			"Error: Target element is not an object."
		);
		return NULL;
	}

	const struct InstanceValue *instance = _instance_value;

	int length = get_expr->property_token->length;

	char *property = allocate(sizeof(char), length + 1);
	strncpy(property, get_expr->property_token->start, length);

	void *ret_value = (void *) instance_get_property(instance, property);
	free(property);

	if(NULL == ret_value) {
		i_raise_error(
			get_expr->property_token,
			"Error: Undefined property."
		);
		return NULL;
	}

	delete((void *) instance);

	return copy_value(ret_value);
}



// method to perform calculation for '+=', '-=', '*=', '/=', '%=' operands

void *perf_assign_calculation(
	const uint32_t ordinal,
	const void *expr,
	const void *left_value, const void *right_value
) {
	assert(is_ast_node(expr));

	void *result = NULL;

	if(ordinal == TOKEN_TYPE_PLUS_EQUAL.ordinal)
		result = add_values(left_value, right_value);
	
	else if(ordinal == TOKEN_TYPE_MINUS_EQUAL.ordinal)
		result = sub_values(left_value, right_value);

	else if(ordinal == TOKEN_TYPE_STAR_EQUAL.ordinal)
		result = mul_values(left_value, right_value);

	else if((ordinal == TOKEN_TYPE_SLASH_EQUAL.ordinal) || (ordinal == TOKEN_TYPE_PERCENT_EQUAL.ordinal)) {
		if(VALUE_TYPE_INTEGER == GET_VALUE_TYPE(right_value)) {
			if(0 == VALUE_AS_INT(right_value)) {
				i_raise_error_expr(
					interpreter, expr,
					"RuntimeError: Division by zero."
				);
			}
		}
		else if(VALUE_TYPE_DOUBLE == GET_VALUE_TYPE(right_value)) {
			if(0.0 == VALUE_AS_DOUBLE(right_value)) {
				i_raise_error_expr(
					interpreter, expr,
					"RuntimeError: Division by zero."
				);
			}
		}

		if(ordinal == TOKEN_TYPE_SLASH_EQUAL.ordinal) {
			result = div_values(left_value, right_value);
		} else {
			result = mod_values(left_value, right_value);
		}
	}

	return result;
}



static void *i_evaluate_id_assign_expr(const void *ast_visitor, const void *_id_assign_expr) {
	const struct IdentifierAssignExpr *id_assign_expr = _id_assign_expr;
	assert(IS_IDENTIFIER_ASSIGN_EXPR(id_assign_expr));

	const void *id_symbol = id_assign_expr->symbol;

	const void *cur_var_value = rstack_get_value(interpreter->runtime_stack, id_symbol);
	void *right_value = VISIT_NODE(ast_visitor, id_assign_expr->right_expr);

	const uint8_t ordinal = id_assign_expr->equal_token->token_type.ordinal;

	if(ordinal == TOKEN_TYPE_EQUAL.ordinal) {
		rstack_set_value(interpreter->runtime_stack, id_symbol, copy_value(right_value));
		return right_value;
	}

	// else {

	void *res_value = perf_assign_calculation(
		id_assign_expr->equal_token->token_type.ordinal,
		id_assign_expr,
		cur_var_value, right_value
	);

	if(NULL == res_value) {
		i_raise_error_expr(
			interpreter, id_assign_expr,
			"RuntimeError: Incompatible operands for operator '%.*s'.",
			id_assign_expr->equal_token->length, id_assign_expr->equal_token->start
		);
		return NULL;
	}

	rstack_set_value(interpreter->runtime_stack, id_symbol, copy_value(res_value));
	delete(right_value);

	return res_value;

	// }
}



static void *i_evaluate_ls_assign_expr(const void *ast_visitor, const void *_ls_assign_expr) {
	const struct ListSubscriptAssignExpr *ls_assign_expr = _ls_assign_expr;
	assert(IS_LIST_SUBSCRIPT_ASSIGN_EXPR(ls_assign_expr));

	void *_list_value = VISIT_NODE(interpreter->ast_visitor, ls_assign_expr->left_expr);

	if(!IS_LIST_VALUE(_list_value)) {
		i_raise_error_expr(
			interpreter,
			ls_assign_expr->left_expr,
			"Error: Subscripted element is not a list."
		);

		return NULL;
	}

	struct ListValue *list_value = _list_value;
	struct Value *index_value = (struct Value *) VISIT_NODE(ast_visitor, ls_assign_expr->index_expr);

	if(VALUE_TYPE_INTEGER != GET_VALUE_TYPE(index_value)) {
		i_raise_error_expr(
			interpreter,
			ls_assign_expr->index_expr,
			"Error: Subscripting index should be an integer."
		);
		return NULL;
	}

	int index = VALUE_AS_INT(index_value);
	delete(index_value);

	if(
		(index < ((int) (-1 * list_value->count))) ||
		(index >= ((int) list_value->count))
	) {
		i_raise_error_expr(
			interpreter,
			ls_assign_expr->index_expr,
			"Error: Index out of bounds."
		);
		return NULL;
	}

	index = (index >= 0) ? index : (((int) list_value->count) + ((int) index));

	void *right_value = VISIT_NODE(ast_visitor, ls_assign_expr->right_expr);
	const uint32_t ordinal = ls_assign_expr->equal_token->token_type.ordinal;


	if(ordinal == TOKEN_TYPE_EQUAL.ordinal) {
		void *old_value = list_value->values[index];
		void *new_value = right_value;

		delete(old_value);
		list_value->values[index] = new_value;

		delete(list_value);

		return copy_value(new_value);
	}
	
	// else {

	void *old_value = list_value->values[index];

	void *new_value = perf_assign_calculation(
		ls_assign_expr->equal_token->token_type.ordinal,
		ls_assign_expr,
		old_value, right_value
	);

	if(NULL == new_value) {
		i_raise_error_expr(
			interpreter, ls_assign_expr,
			"RuntimeError: Incompatible operands for operator '%.*s'.",
			ls_assign_expr->equal_token->length, ls_assign_expr->equal_token->start
		);
		return NULL;
	}

	delete(old_value);
	delete(right_value);

	delete(list_value);

	list_value->values[index] = new_value;

	return copy_value(new_value);

	// }
}



static void *i_evaluate_set_expr(const void *ast_visitor, const void *_set_expr) {
	const struct SetExpr *set_expr = _set_expr;
	assert(IS_SET_EXPR(set_expr));

	return NULL;
}






static void *i_evaluate_list_expr(const void *ast_visitor, const void *_list_expr) {
	const struct ListExpr *list_expr = _list_expr;
	assert(IS_LIST_EXPR(list_expr));

	void **values = allocate(sizeof(void *), list_expr->expr_count);

	for(uint32_t i = 0; i < list_expr->expr_count; i++) {
		values[i] = VISIT_NODE(interpreter->ast_visitor, list_expr->exprs[i]);
	}

	return new(ListValue, list_expr->expr_count, values);

	/*
	enum ValueType list_value_type;

	if(list_expr->expr_count > 0) {
		struct Value *value = VISIT_NODE(ast_visitor, list_expr->exprs[0]);
		list_value_type	= value->value_type;

		void *_values = NULL;
		
		switch(list_value_type) {
			case VALUE_TYPE_BOOL: {
				_values = allocate(sizeof(uint8_t), list_expr->expr_count);
				
				uint8_t *values = _values;
				*values = VALUE_AS_INT(value);
			}
			case VALUE_TYPE_INTEGER: {
				_values = allocate(sizeof(int), list_expr->expr_count);
				
				int *values = _values;
				*values = VALUE_AS_INT(value);
			}
			case VALUE_TYPE_DOUBLE: {
				_values = allocate(sizeof(double), list_expr->expr_count);
				
				double *values = _values;
				*values = VALUE_AS_DOUBLE(value);
			}
			case VALUE_TYPE_STRING: {
				_values = allocate(sizeof(char *), list_expr->expr_count);
				
				char **values = _values;
				values[0] = VALUE_AS_STRING(value);
			}
		}


		for(uint32_t i = 1; i < list_expr->expr_count; i++) {
			value = VISIT_NODE(ast_visitor, list_expr->exprs[i]);

			if(value->value_type != list_value_type) {
				i_raise_error(NULL, "Error: Illegal value in list.");
			}

			switch(list_value_type) {
				case VALUE_TYPE_BOOL: {
					uint8_t *values = _values;
					*(values + i) = VALUE_AS_INT(value);
				}
				case VALUE_TYPE_INTEGER: {
					int *values = _values;
					*(values + i) = VALUE_AS_INT(value);
				}
				case VALUE_TYPE_DOUBLE: {
					double *values = _values;
					*(values + i) = VALUE_AS_DOUBLE(value);
				}
				case VALUE_TYPE_STRING: {
					char **values = _values;
					values[i] = VALUE_AS_STRING(value);
				}
			}
		}
	}
	*/
}



static void *i_evaluate_list_subscript_expr(const void *ast_visitor, const void *_ls_expr) {
	const struct ListSubscriptExpr *ls_expr = _ls_expr;
	assert(IS_LIST_SUBSCRIPT_EXPR(ls_expr));

	void *_list_value = VISIT_NODE(interpreter->ast_visitor, ls_expr->left_expr);

	if(!IS_LIST_VALUE(_list_value)) {
		i_raise_error_expr(
			interpreter,
			ls_expr->left_expr,
			"Error: Subscripted element is not a list."
		);

		return NULL;
	}

	struct ListValue *list_value = _list_value;
	struct Value *index_value = (struct Value *) VISIT_NODE(ast_visitor, ls_expr->index_expr);

	if(VALUE_TYPE_INTEGER != GET_VALUE_TYPE(index_value)) {
		i_raise_error_expr(
			interpreter,
			ls_expr->index_expr,
			"Error: Subscripting index should be an integer."
		);
		return NULL;
	}

	int index = VALUE_AS_INT(index_value);
	delete(index_value);

	if(
		(index < ((int) (-1 * list_value->count))) ||
		(index >= ((int) list_value->count))
	) {
		i_raise_error_expr(
			interpreter,
			ls_expr->index_expr,
			"Error: Index out of bounds."
		);
		return NULL;
	}

	index = (index >= 0) ? index : (((int) list_value->count) + ((int) index));

	delete(list_value);

	return copy_value(list_value->values[index]);
}



void interpret(void *_self) {
	// time_t start_time;
	// time(&start_time);




	struct Interpreter *self = _self;
	assert(self && (self->class == Interpreter));

	interpreter = self;

	// predefined's stack frame
	void *predef_symbol_table = (void *) symtabstack_get_local_table(self->symbol_table_stack);
	void *predef_stack_frame = (void *) rstack_push(self->runtime_stack, predef_symbol_table);

	define_native_funs((void *) self->symbol_table_stack, predef_stack_frame);

	VISIT_NODE(self->ast_visitor, self->program_ast);

	rstack_pop(self->runtime_stack);	// predef_stack_frame




	// time_t end_time;
	// time(&end_time);

	// printf("Total Execution time: %ds\n", difftime(end_time, start_time));
}


#undef VISIT_NODE

