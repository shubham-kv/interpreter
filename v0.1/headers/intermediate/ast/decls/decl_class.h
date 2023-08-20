#if !defined(decl_class_h)
	#define decl_class_h

	#include <stdint.h>

	struct ClassDecl {
		const void *type;

		const void *symbol;
		void *symbol_table;

		uint32_t property_count;

		// list of symbols for maintaining sequences
		// already declared within the symbol table of class
		void **properties;

		// hashtable of methods
		// method's definition remains in symbol_table while the ast in this hashtable
		void *methods;
	};

	extern const void *ClassDecl;

	#define IS_CLASS_DECL(obj) ((obj) && (((struct ClassDecl *) obj)->type == ClassDecl))

#endif // decl_class_h

