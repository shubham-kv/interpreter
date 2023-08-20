#if !defined(stmt_block_h)
	#define stmt_block_h

	struct BlockStmt {
		// Pointer to the class' specification.
		const void *type;

		// the symbol table of the block statement
		void *symbol_table;

		// Pointer to an array of pointers to statments.
		void **child_stmts;
	};

	extern const void *BlockStmt;

	#define IS_BLOCK_STMT(obj) ((obj) && (((struct BlockStmt *) obj)->type == BlockStmt))

#endif // stmt_block_h
