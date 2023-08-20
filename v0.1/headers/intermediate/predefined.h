#if !defined(predefined_h)
	#define predefined_h

	void declare_native_funs(void *predef_symbol_table);

	void define_native_funs(void *symbol_table_stack, void *stack_frame);

#endif // predefined_h
