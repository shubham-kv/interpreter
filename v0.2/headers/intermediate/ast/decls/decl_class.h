#if !defined(decl_class_h)
	#define decl_class_h

	#include <stdint.h>

	/*
	// v0.1
	struct ClassDecl {
		const void *type;

		const void *symbol;
		void *symbol_table;

		uint32_t property_count;

		// list of symbols for maintaining sequences
		// already declared within the symbol table of class
		void **properties;

		// hashtable of methods
		// method's definition remains in symbol_table while the FunDecl in this hashtable
		void *methods;
	};
	*/

	// v0.2
	struct ClassDecl {
		const void *type;

		const void *symbol;
		void *symbol_table;

		void *super_class_symbol;

		uint32_t property_count;
		void **properties;

		// null terminated list of fun_decls of the class
		void **methods;
	};

	extern const void *ClassDecl;

	#define IS_CLASS_DECL(obj) ((obj) && (((struct ClassDecl *) obj)->type == ClassDecl))
	
	extern const char cur_instance_ptr_name[];
	// extern const char super_name[];

#endif // decl_class_h

