#if !defined(predefined_h)
	#define predefined_h

	#include "symbol_table.h"
	#include "stack_frame.h"

	extern void *predef_symbol_table;
	extern void *predef_stack_frame;

	void init_predef_table();
	void destroy_predef_table();

	void init_predef_frame();
	void destroy_predef_frame();

	// declare native funs within a predefined symbol table
	void declare_native_funs();

	// define native funs within a predefined stack frame
	void define_native_funs(void *symbol_table_stack);

#endif // predefined_h
