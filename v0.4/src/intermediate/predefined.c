#include "predefined.h"

#include <assert.h>

#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "my_memory.h"
#include "object.h"

#include "headers_backend_value.h"

#include "headers_intermediate_symbol.h"

#include "stack_frame.h"


void *predef_symbol_table = NULL;
void *predef_stack_frame = NULL;

static bool declared = false;
static bool defined = false;


static void *execute_out(const void *fun, void **args) {
	print_value(args[0]);
	return NULL;
}

static void *execute_outln(const void *fun, void **args) {
	print_value(args[0]);
	printf("\n");
	return NULL;
}

#define INPUT_BUFFER_SIZE 64
#define STR_INPUT_BUFFER_SIZE 1024

static void *execute_in_int(const void *fun, void **args) {
	int i;
	char *t;
	char buffer[INPUT_BUFFER_SIZE];

	do {
		print_value(args[0]);
		fflush(stdout);

		if(fgets(buffer, INPUT_BUFFER_SIZE, stdin)) {
			i = strtol(buffer, &t, 10);

			if(!((t == buffer) || (*t != '\n'))) {
				break;
			}
		}
	} while(1);

	return new(Value, VALUE_TYPE_INTEGER, i);
}

static void *execute_in_double(const void *fun, void **args) {
	double d;
	char *t;
	char buffer[INPUT_BUFFER_SIZE];

	do {
		print_value(args[0]);
		fflush(stdout);

		if(fgets(buffer, INPUT_BUFFER_SIZE, stdin)) {
			d = strtod(buffer, &t);

			if(!((t == buffer) || (*t != '\n'))) {
				break;
			}
		}
	} while(1);


	return new(Value, VALUE_TYPE_DOUBLE, d);
}

static void *execute_in_str(const void *fun, void **args) {
	char *t;
	char buffer[STR_INPUT_BUFFER_SIZE];

	print_value(args[0]);
	fflush(stdout);

	if(fgets(buffer, STR_INPUT_BUFFER_SIZE, stdin)) {
		size_t len = strlen(buffer);
		char *str = allocate(sizeof(char), len + 1);
		strncpy(str, buffer, len);

		return new(Value, VALUE_TYPE_STRING, str);
	}

	return new(Value, VALUE_TYPE_STRING, allocate(sizeof(char), 1));
}


// Type conversion methods

static void *execute_asInt(const void *fun, void **args) {
	enum ValueType type = GET_VALUE_TYPE(args[0]);

	if(VALUE_TYPE_INTEGER == type) {
		return copy_value(args[0]);
	}
	else if(VALUE_TYPE_DOUBLE == type) {
		double d = VALUE_AS_DOUBLE(args[0]);
		int i = floor(d);
		return new(Value, VALUE_TYPE_INTEGER, i);
	}
	else if(VALUE_TYPE_STRING == type) {
		uint8_t *str = VALUE_AS_STRING(args[0]);
		int i = (uint8_t) str[0];
		return new(Value, VALUE_TYPE_INTEGER, i);
	}

	return NULL;
}

static void *execute_asDouble(const void *fun, void **args) {
	enum ValueType type = GET_VALUE_TYPE(args[0]);

	if(VALUE_TYPE_INTEGER == type) {
		return new(Value, VALUE_TYPE_DOUBLE, floor(VALUE_AS_INT(args[0])));
	}
	else if(VALUE_TYPE_DOUBLE == type) {
		return copy_value(args[0]);
	}

	return NULL;
}

static void *execute_asChar(const void *fun, void **args) {
	enum ValueType type = GET_VALUE_TYPE(args[0]);

	if(VALUE_TYPE_STRING == type) {
		return copy_value(args[0]);
	}
	else if(VALUE_TYPE_INTEGER == type) {
		uint8_t *str = allocate(sizeof(char), 2);
		str[0] = (uint8_t) VALUE_AS_INT(args[0]);
		return new(Value, VALUE_TYPE_STRING, str);
	}

	return NULL;
}










// Math funs

static void *execute_abs(const void *fun, void **args) {
	enum ValueType value_type = GET_VALUE_TYPE(args[0]);

	if(VALUE_TYPE_INTEGER == value_type) {
		int i = VALUE_AS_INT(args[0]);
		return new(Value, VALUE_TYPE_INTEGER, abs(i));
	}
	else if(VALUE_TYPE_DOUBLE == value_type) {
		double d = VALUE_AS_DOUBLE(args[0]);
		return new(Value, VALUE_TYPE_INTEGER, abs(d));
	}
	else {
		return NULL;
	}
}

static void *execute_ceil(const void *fun, void **args) {
	enum ValueType value_type = GET_VALUE_TYPE(args[0]);

	if(VALUE_TYPE_INTEGER == value_type) {
		int i = VALUE_AS_INT(args[0]);
		return new(Value, VALUE_TYPE_DOUBLE, ceil(i));
	}
	else if(VALUE_TYPE_DOUBLE == value_type) {
		double d = VALUE_AS_DOUBLE(args[0]);
		return new(Value, VALUE_TYPE_DOUBLE, ceil(d));
	}
	else {
		return NULL;
	}
}

static void *execute_floor(const void *fun, void **args) {
	enum ValueType value_type = GET_VALUE_TYPE(args[0]);

	if(VALUE_TYPE_INTEGER == value_type) {
		int i = VALUE_AS_INT(args[0]);
		return new(Value, VALUE_TYPE_DOUBLE, floor(i));
	}
	else if(VALUE_TYPE_DOUBLE == value_type) {
		double d = VALUE_AS_DOUBLE(args[0]);
		return new(Value, VALUE_TYPE_DOUBLE, floor(d));
	}
	else {
		return NULL;
	}
}


static void *execute_sqrt(const void *fun, void **args) {
	enum ValueType value_type = GET_VALUE_TYPE(args[0]);

	if(VALUE_TYPE_INTEGER == value_type) {
		int i = VALUE_AS_INT(args[0]);
		double res = sqrt(i);

		return new(Value, VALUE_TYPE_DOUBLE, res);
	}
	else if(VALUE_TYPE_DOUBLE == value_type) {
		double d = VALUE_AS_DOUBLE(args[0]);
		double res = sqrt(d);

		return new(Value, VALUE_TYPE_DOUBLE, res);
	}
	else {
		return NULL;
	}
}


static void *execute_pow(const void *fun, void **args) {
	enum ValueType type1 = GET_VALUE_TYPE(args[0]);
	enum ValueType type2 = GET_VALUE_TYPE(args[1]);

	if(VALUE_TYPE_INTEGER == type1) {
		int base = VALUE_AS_INT(args[0]);
		double res = 0;

		if(VALUE_TYPE_INTEGER == type2) {
			int exponent = VALUE_AS_INT(args[1]);
			res = pow(base, exponent);
		}
		else if(VALUE_TYPE_DOUBLE == type2) {
			double exponent = VALUE_AS_DOUBLE(args[1]);
			res = pow(base, exponent);
		}
		else {
			return NULL;
		}

		return new(Value, VALUE_TYPE_DOUBLE, res);
	}
	else if(VALUE_TYPE_DOUBLE == type1) {
		double base = VALUE_AS_DOUBLE(args[0]);
		double res = 0;

		if(VALUE_TYPE_INTEGER == type2) {
			int exponent = VALUE_AS_INT(args[1]);
			res = pow(base, exponent);
		}
		else if(VALUE_TYPE_DOUBLE == type2) {
			double exponent = VALUE_AS_DOUBLE(args[1]);
			res = pow(base, exponent);
		}
		else {
			return NULL;
		}

		return new(Value, VALUE_TYPE_DOUBLE, res);
	}
	else {
		return NULL;
	}
}



// String methods

static void *execute_strLen(const void *fun, void **args) {
	enum ValueType type1 = GET_VALUE_TYPE(args[0]);

	if(VALUE_TYPE_STRING == type1) {
		char *str = VALUE_AS_STRING(args[0]);
		return new(Value, VALUE_TYPE_INTEGER, strlen(str));
	}

	return NULL;
}

static void *execute_charAt(const void *fun, void **args) {
	enum ValueType type1 = GET_VALUE_TYPE(args[0]);
	enum ValueType type2 = GET_VALUE_TYPE(args[1]);
	
	if(VALUE_TYPE_STRING == type1 && VALUE_TYPE_INTEGER == type2) {
		char *str = VALUE_AS_STRING(args[0]);
		int index = VALUE_AS_INT(args[1]);
		int len = strlen(str);

		if((index < (-1 * len)) || (index >= len)) {
			return NULL;
		}

		index = (index < 0) ? (len + index) : index;

		char *str_val = allocate(sizeof(char), 2);
		strncpy(str_val, str + index, 1);

		return new(Value, VALUE_TYPE_STRING, str_val);
	}

	return NULL;
}

static void *execute_toUpper(const void *fun, void **args) {
	enum ValueType type1 = GET_VALUE_TYPE(args[0]);
	
	if(VALUE_TYPE_STRING == type1) {
		char *str = VALUE_AS_STRING(args[0]);
		int len = strlen(str);

		char *val = allocate(sizeof(char), len + 1);

		for(int i = 0; i < len; i++)
			val[i] = toupper(str[i]);

		return new(Value, VALUE_TYPE_STRING, val);
	}

	return NULL;
}

static void *execute_toLower(const void *fun, void **args) {
	enum ValueType type1 = GET_VALUE_TYPE(args[0]);
	
	if(VALUE_TYPE_STRING == type1) {
		char *str = VALUE_AS_STRING(args[0]);
		int len = strlen(str);
		char *val = allocate(sizeof(char), len + 1);

		for(int i = 0; i < len; i++)
			val[i] = tolower(str[i]);

		return new(Value, VALUE_TYPE_STRING, val);
	}

	return NULL;
}


// list funs
static void *execute_len(const void *fun, void **args) {
	if(IS_LIST_VALUE(args[0])) {
		const struct ListValue *list = args[0];
		return new(Value, VALUE_TYPE_INTEGER, list->count);
	}
	return NULL;
}

// push(myList, element);
static void *execute_push(const void *fun, void **args) {
	if(IS_LIST_VALUE(args[0])) {
		struct ListValue *list = args[0];
		list->values = reallocate(list->values, sizeof(void **), ++list->count);
		list->values[list->count - 1] = copy_value(args[1]);
	}
	return NULL;
}

// pop(myList);
static void *execute_pop(const void *fun, void **args) {
	if(IS_LIST_VALUE(args[0])) {
		struct ListValue *list = args[0];

		void *popped_value = list->values[list->count - 1];
		list->values = reallocate(list->values, sizeof(void **), --list->count);

		return popped_value;
	}
	return NULL;
}

// [ 'Foo', 'Bar', 'Fib', 'Anne', 'Ellie']
// insert(list, 1, "Punk");
// [ 'Foo', 'Bar', 'Fib', 'Anne', 'Ellie', NULL ]

static void *execute_insert(const void *fun, void **args) {
	if(IS_LIST_VALUE(args[0]) && IS_VALUE(args[1])) {
		struct ListValue *list = args[0];
		const int index = VALUE_AS_INT(args[1]);

		if(index < 0 || index > list->count)
			return NULL;

		list->values = reallocate(list->values, sizeof(void **), ++list->count);

		for(int i = list->count - 1; i > index; i--) {
			list->values[i] = list->values[i - 1];
		}

		list->values[index] = copy_value(args[2]);
		return copy_value(args[2]);
	}
	return NULL;
}







typedef void *(*Executor_fun)(const void *, void **);


// declare fun within symbol table
static void declare_fun(
	void *predef_symbol_table,
	const char *fun_name
) {
	assert(IS_SYMBOL_TABLE(predef_symbol_table));

	char *fun_id = allocate(sizeof(char), strlen(fun_name) + 1);
	strncpy(fun_id, fun_name, strlen(fun_name));

	// void *fun_symbol = new(Symbol, fun_id, predef_symbol_table, DEF_TYPE_NATIVE_FUN);
	void *fun_symbol = new(Symbol, fun_id, predef_symbol_table, DEF_TYPE_FUN, NULL);
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

	assert(IS_STACK_FRAME(predef_stack_frame));

	const void *fun_symbol = symtabstack_lookup(symbol_table_stack, fun_name);
	assert(IS_SYMBOL(fun_symbol));

	const void *fun_symbol_table = symtabstack_push(symbol_table_stack);	// fun_symbol_table
	void **param_symbols = allocate(sizeof(void *), param_count);

	for(uint32_t i = 0; i < param_count; i++) {
		char *param_id = allocate(sizeof(char), strlen(fun_params[i]) + 1);
		strcpy(param_id, fun_params[i]);

		void *param_symbol = new(Symbol, param_id, fun_symbol_table, DEF_TYPE_VAR, NULL);
		symbol_table_define(fun_symbol_table, param_id, param_symbol);

		param_symbols[i] = param_symbol;
	}
	
	symtabstack_pop(symbol_table_stack);	// fun_symbol_table

	const void *fun_value = new(FunValue, FUNCTION_TYPE_FUN, NULL, fun_symbol, fun_symbol_table, param_count, param_symbols, executor_fun, NULL);
	sframe_put_value(predef_stack_frame, fun_name, fun_value);
}


void init_predef_table() {
	if(NULL == predef_symbol_table) {
		predef_symbol_table = new(SymbolTable, 0);
	}
}

void destroy_predef_table() {
	delete((void *) predef_symbol_table);
}


void init_predef_frame() {
	if(NULL == predef_stack_frame) {
		predef_stack_frame = new(StackFrame, 0, NULL);
	}
}

void destroy_predef_frame() {
	delete((void *) predef_stack_frame);
}


void declare_native_funs() {
	if(declared) {
		return;
	}
	// else {
		declare_fun(predef_symbol_table, "out");
		declare_fun(predef_symbol_table, "outln");

		declare_fun(predef_symbol_table, "in_int");
		declare_fun(predef_symbol_table, "in_double");
		declare_fun(predef_symbol_table, "in_str");

		// Type conversion methods
		declare_fun(predef_symbol_table, "asInt");
		declare_fun(predef_symbol_table, "asDouble");
		declare_fun(predef_symbol_table, "asChar");

		// math funs
		declare_fun(predef_symbol_table, "abs");
		declare_fun(predef_symbol_table, "ceil");
		declare_fun(predef_symbol_table, "floor");
		declare_fun(predef_symbol_table, "sqrt");
		declare_fun(predef_symbol_table, "pow");

		// string funs
		declare_fun(predef_symbol_table, "strLen");
		declare_fun(predef_symbol_table, "charAt");
		declare_fun(predef_symbol_table, "toUpper");
		declare_fun(predef_symbol_table, "toLower");


		// list funs
		declare_fun(predef_symbol_table, "len");

		// push an element into the list, modifies original and returns it
		declare_fun(predef_symbol_table, "push");

		// pops the last element from the list, modifies original and returns the popped element
		declare_fun(predef_symbol_table, "pop");

		// inserts an element into the list at the specified index
		declare_fun(predef_symbol_table, "insert");

		declared = true;
	// }
}


void define_native_funs(void *symbol_table_stack) {
	assert(IS_SYMBOL_TABLE_STACK(symbol_table_stack));

	if(defined) {
		return;
	}
	// else {

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

		{
			const char *fun_name = "in_str";
			const uint32_t param_count = 1;
			const char *const parameters[] = { "prompt" };

			define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_in_str);
		}

		// Type conversion funs
		{
			const char *fun_name = "asInt";
			const uint32_t param_count = 1;
			const char *const parameters[] = { "input" };

			define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_asInt);
		}

		{
			const char *fun_name = "asDouble";
			const uint32_t param_count = 1;
			const char *const parameters[] = { "input" };

			define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_asDouble);
		}

		{
			const char *fun_name = "asChar";
			const uint32_t param_count = 1;
			const char *const parameters[] = { "input" };

			define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_asChar);
		}





		// Math funs

		{
			const char *fun_name = "abs";
			const uint32_t param_count = 1;
			const char *const parameters[] = { "number" };

			define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_abs);
		}

		{
			const char *fun_name = "ceil";
			const uint32_t param_count = 1;
			const char *const parameters[] = { "number" };

			define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_ceil);
		}

		{
			const char *fun_name = "floor";
			const uint32_t param_count = 1;
			const char *const parameters[] = { "number" };

			define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_floor);
		}

		{
			const char *fun_name = "sqrt";
			const uint32_t param_count = 1;
			const char *const parameters[] = { "number" };

			define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_sqrt);
		}

		{
			const char *fun_name = "pow";
			const uint32_t param_count = 2;
			const char *const parameters[] = { "base", "exponent" };

			define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_pow);
		}




		// String funs
		{
			const char *fun_name = "strLen";
			const uint32_t param_count = 1;
			const char *const parameters[] = { "string" };
			define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_strLen);
		}

		{
			const char *fun_name = "charAt";
			const uint32_t param_count = 2;
			const char *const parameters[] = { "string", "index" };
			define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_charAt);
		}

		{
			const char *fun_name = "toUpper";
			const uint32_t param_count = 1;
			const char *const parameters[] = { "string" };
			define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_toUpper);
		}

		{
			const char *fun_name = "toLower";
			const uint32_t param_count = 1;
			const char *const parameters[] = { "string" };
			define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_toLower);
		}



		// list funs
		{
			const char *fun_name = "len";
			const uint32_t param_count = 1;
			const char *const parameters[] = { "list" };
			define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_len);
		}

		{
			const char *fun_name = "push";
			const uint32_t param_count = 2;
			const char *const parameters[] = { "list", "element" };
			define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_push);
		}

		{
			const char *fun_name = "pop";
			const uint32_t param_count = 1;
			const char *const parameters[] = { "list" };
			define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_pop);
		}

		{
			const char *fun_name = "insert";
			const uint32_t param_count = 3;
			const char *const parameters[] = { "list", "atIndex", "element" };
			define_fun(symbol_table_stack, predef_stack_frame, fun_name, param_count, parameters, execute_insert);
		}


		defined = true;
	// }
}

