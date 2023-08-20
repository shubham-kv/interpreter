#include "predefined.h"

#include <assert.h>

#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "object.h"
#include "memory.h"

#include "value_object.h"
#include "value.h"
#include "value_fun.h"

#include "def_type.h"
#include "symbol.h"
#include "symbol_table.h"
#include "symbol_table_stack.h"

#include "stack_frame.h"


typedef void *(*Executor_fun)(const void *, void **);


// static void init_fun(
// 	void *symbol_table_stack,
// 	const char *fun_name,
// 	uint32_t param_count,
// 	const char *const *fun_params,
// 	Executor_fun executor_fun
// ) {

// 	assert(IS_SYMBOL_TABLE_STACK(symbol_table_stack));


// 	char *fun_id = allocate(sizeof(char), strlen(fun_name) + 1);
// 	strncpy(fun_id, fun_name, strlen(fun_name));

// 	const void *fun_symbol_table = symtabstack_push(symbol_table_stack);	// fun_symbol_table
// 	void **param_symbols = allocate(sizeof(void *), param_count);

// 	for(uint32_t i = 0; i < param_count; i++) {
// 		char *param_id = allocate(sizeof(char), strlen(fun_params[i]) + 1);
// 		strcpy(param_id, fun_params[i]);

// 		void *param_symbol = new(Symbol, param_id, fun_symbol_table, DEF_TYPE_VARIABLE);
// 		symbol_table_define(fun_symbol_table, param_id, param_symbol);

// 		param_symbols[i] = param_symbol;
// 	}
	
// 	symtabstack_pop(symbol_table_stack);	// fun_symbol_table


// 	const void *predef_table = symtabstack_get_local_table(symbol_table_stack);

// 	void *fun_symbol = new(Symbol, fun_id, predef_table, DEF_TYPE_NATIVE_FUN);

// 	void *fun_value = new(FunValue, fun_symbol, fun_symbol_table, param_count, param_symbols, executor_fun);
// 	symbol_set_attr(fun_symbol, SYM_ATTR_KEY_FUN, fun_value);
// 	symbol_table_define(predef_table, fun_id, fun_symbol);
// }



// declare fun within symbol table

static void declare_fun(
	void *predef_symbol_table,
	const char *fun_name
) {
	// assert(IS_SYMBOL_TABLE(predef_symbol_table));

	char *fun_id = allocate(sizeof(char), strlen(fun_name) + 1);
	strncpy(fun_id, fun_name, strlen(fun_name));

	void *fun_symbol = new(Symbol, fun_id, predef_symbol_table, DEF_TYPE_NATIVE_FUN);
	symbol_table_define(predef_symbol_table, fun_id, fun_symbol);
}


// define fun within stack frame

static void define_fun(
	void *symbol_table_stack,
	void *predef_stack_frame,
	const char *fun_name,
	uint32_t param_count,
	const char *const *fun_params,
	Executor_fun executor_fun
) {

	// assert(IS_STACK_FRAME(predef_stack_frame));

	const void *fun_symbol = symtabstack_lookup(symbol_table_stack, fun_name);
	assert(IS_SYMBOL(fun_symbol));

	const void *fun_symbol_table = symtabstack_push(symbol_table_stack);	// fun_symbol_table
	void **param_symbols = allocate(sizeof(void *), param_count);

	for(uint32_t i = 0; i < param_count; i++) {
		char *param_id = allocate(sizeof(char), strlen(fun_params[i]) + 1);
		strcpy(param_id, fun_params[i]);

		void *param_symbol = new(Symbol, param_id, fun_symbol_table, DEF_TYPE_VARIABLE);
		symbol_table_define(fun_symbol_table, param_id, param_symbol);

		param_symbols[i] = param_symbol;
	}
	
	symtabstack_pop(symbol_table_stack);	// fun_symbol_table


	const void *predef_table = symtabstack_get_local_table(symbol_table_stack);

	const void *fun_value = new(FunValue, fun_symbol, fun_symbol_table, param_count, param_symbols, executor_fun, NULL);
	sframe_put_value(predef_stack_frame, fun_name, fun_value);
}







static void *execute_out(const void *fun, void **args) {
	print_value(args[0]);
	// not deleteing cuz when the frame is popped of, it automatically gets deleted
	// delete(args[0]);

	return NULL;
}

static void *execute_outln(const void *fun, void **args) {
	print_value(args[0]);
	printf("\n");
	// delete(args[0]);

	return NULL;
}

static void *execute_in_int(const void *fun, void **args) {
	if(NULL != args && NULL != args[0]) {
		print_value(args[0]);
	}

	int scanned_int;
	scanf("%d", &scanned_int);

	return new(Value, VALUE_TYPE_INTEGER, scanned_int);
}

static void *execute_in_double(const void *fun, void **args) {
	if(NULL != args && NULL != args[0]) {
		print_value(args[0]);
	}

	// got to fix the problem with this scanning
	double scanned_double;
	scanf("%lf", &scanned_double);

	return new(Value, VALUE_TYPE_DOUBLE, scanned_double);
}


/*
void init_native_funs(void *symbol_table_stack) {
	assert(IS_SYMBOL_TABLE_STACK(symbol_table_stack));

	{
		const char *fun_name = "out";
		const uint32_t param_count = 1;
		const char *const parameters[] = { "value" };

		init_fun(symbol_table_stack, fun_name, param_count, parameters, execute_out);
	}

	{
		const char *fun_name = "outln";
		const uint32_t param_count = 1;
		const char *const parameters[] = { "value" };

		init_fun(symbol_table_stack, fun_name, param_count, parameters, execute_outln);
	}

	{
		const char *fun_name = "in_int";
		const uint32_t param_count = 1;
		const char *const parameters[] = { "prompt" };

		init_fun(symbol_table_stack, fun_name, param_count, parameters, execute_in_int);
	}

	{
		const char *fun_name = "in_double";
		const uint32_t param_count = 1;
		const char *const parameters[] = { "prompt" };

		init_fun(symbol_table_stack, fun_name, param_count, parameters, execute_in_double);
	}
}
*/



void declare_native_funs(void *predef_symbol_table) {
	assert(IS_SYMBOL_TABLE(predef_symbol_table));

	declare_fun(predef_symbol_table, "out");
	declare_fun(predef_symbol_table, "outln");
	declare_fun(predef_symbol_table, "in_int");
	declare_fun(predef_symbol_table, "in_double");
}




void define_native_funs(void *symbol_table_stack, void *predef_stack_frame) {
	assert(IS_SYMBOL_TABLE_STACK(symbol_table_stack));
	assert(IS_STACK_FRAME(predef_stack_frame));

	{
		const char *fun_name = "out";
		const uint32_t param_count = 1;
		const char *const parameters[] = { "value" };

		define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_out);
	}

	{
		const char *fun_name = "outln";
		const uint32_t param_count = 1;
		const char *const parameters[] = { "value" };

		define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_outln);
	}

	{
		const char *fun_name = "in_int";
		const uint32_t param_count = 1;
		const char *const parameters[] = { "prompt" };

		define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_in_int);
	}

	{
		const char *fun_name = "in_double";
		const uint32_t param_count = 1;
		const char *const parameters[] = { "prompt" };

		define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_in_double);
	}
}



