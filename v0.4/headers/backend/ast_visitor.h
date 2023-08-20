#if !defined(ast_visitor_h)
	#define ast_visitor_h

	#include <stdbool.h>

	struct AstVisitor {
		const void *type;

		// program
		void *(*visit_program_ast)	(const void *ast_vititor, const void *program_ast);

		void *(*visit_import_stmt)	(const void *ast_vititor, const void *import_stmt);


		// declarations
		void *(*visit_var_decl)		(const void *ast_vititor, const void *var_decl);

 		void *(*visit_fun_decl)		(const void *ast_vititor, const void *fun_decl);
 
 		void *(*visit_class_decl)		(const void *ast_vititor, const void *class_decl);


		// statements
		void *(*visit_expr_stmt)		(const void *ast_vititor, const void *expr_stmt);

		void *(*visit_if_stmt)			(const void *ast_vititor, const void *if_stmt);

		void *(*visit_when_stmt)		(const void *ast_vititor, const void *when_stmt);

		void *(*visit_loop_stmt)		(const void *ast_vititor, const void *loop_stmt);

		void *(*visit_repeat_stmt)		(const void *ast_vititor, const void *repeat_stmt);

		void *(*visit_block_stmt)		(const void *ast_vititor, const void *block_stmt);

		void *(*visit_break_stmt)		(const void *ast_vititor, const void *break_stmt);

		void *(*visit_continue_stmt)	(const void *ast_vititor, const void *continue_stmt);

		void *(*visit_ret_stmt)			(const void *ast_vititor, const void *ret_stmt);


		// expressions
		void *(*visit_identifier_expr)	(const void *ast_visitor, const void *identifier_expr);

		void *(*visit_literal_expr)		(const void *ast_visitor, const void *literal_expr);

		void *(*visit_unary_expr)		(const void *ast_visitor, const void *unary_expr);

		void *(*visit_binary_expr)		(const void *ast_visitor, const void *binary_expr);

		void *(*visit_logical_expr)		(const void *ast_visitor, const void *logical_expr);
		
		void *(*visit_bitwise_expr)		(const void *ast_visitor, const void *bitwise_expr);

		void *(*visit_group_expr)		(const void *ast_visitor, const void *group_expr);

		void *(*visit_call_expr)		(const void *ast_visitor, const void *call_expr);

		void *(*visit_super_constructor_call_expr)	(const void *ast_visitor, const void *call_expr);

		void *(*visit_get_expr)			(const void *ast_visitor, const void *get_expr);

		void *(*visit_super_get_expr)	(const void *ast_visitor, const void *super_get_expr);


		void *(*visit_id_assign_expr)	(const void *ast_visitor, const void *id_assign_expr);

		void *(*visit_ls_assign_expr)	(const void *ast_visitor, const void *ls_assign_expr);

		void *(*visit_set_expr)			(const void *ast_visitor, const void *set_expr);


		void *(*visit_list_expr)		(const void *ast_visitor, const void *list_expr);

		void *(*visit_list_subscript_expr)	(const void *ast_visitor, const void *lea_expr);
	};

	extern const void *AstVisitor;

	#define IS_AST_VISITOR(ptr) ((ptr) && (((const struct AstVisitor *) ptr)->type == AstVisitor))

#endif // ast_visitor_h

