#if !defined(ast_node_headers_h)
	#define ast_node_headers_h

	#include "program_ast.h"

	#include "decl_var.h"
	#include "decl_func.h"
	#include "decl_class.h"

	#include "stmt_expr.h"
	#include "stmt_if.h"
	#include "stmt_when.h"
	#include "stmt_loop.h"
	#include "stmt_repeat.h"
	#include "stmt_block.h"

	#include "stmt_break.h"
	#include "stmt_continue.h"
	#include "stmt_ret.h"

	#include "expr_identifier.h"
	#include "expr_literal.h"
	#include "expr_unary.h"
	#include "expr_binary.h"
	#include "expr_logical.h"
	#include "expr_group.h"
	#include "expr_call.h"
	#include "expr_get.h"

	// #include "expr_assign.h"
	#include "expr_assign_id.h"
	#include "expr_assign_list_subscript.h"
	#include "expr_set.h"

	#include "expr_list.h"
	#include "expr_list_subscript.h"

#endif // ast_node_headers_h
