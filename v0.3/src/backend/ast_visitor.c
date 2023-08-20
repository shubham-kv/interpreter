#include "ast_visitor.h"

#include <assert.h>

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>

#include "type.h"


static void *ast_visitor_constructor(void *self, va_list *ap);
static void *ast_visitor_destructor(void *self);

static const struct Type _type_ast_visitor = {
	.size = sizeof(struct AstVisitor),
	.constructor = ast_visitor_constructor,
	.destructor = ast_visitor_destructor
};

const void *AstVisitor = &_type_ast_visitor;

static void *ast_visitor_constructor(void *_self, va_list *ap) {
	struct AstVisitor *self = _self;
	assert(IS_AST_VISITOR(self));

	typedef void *(*visitor_fun)(const void *, const void *);

	self->visit_program_ast = va_arg(*ap, visitor_fun);

	self->visit_var_decl	= va_arg(*ap, visitor_fun);
	self->visit_fun_decl	= va_arg(*ap, visitor_fun);
	self->visit_class_decl	= va_arg(*ap, visitor_fun);

	self->visit_expr_stmt 	= va_arg(*ap, visitor_fun);

	self->visit_if_stmt 	= va_arg(*ap, visitor_fun);
	self->visit_when_stmt 	= va_arg(*ap, visitor_fun);

	self->visit_loop_stmt 	= va_arg(*ap, visitor_fun);
	self->visit_repeat_stmt = va_arg(*ap, visitor_fun);

	self->visit_block_stmt 	= va_arg(*ap, visitor_fun);

	self->visit_break_stmt 		= va_arg(*ap, visitor_fun);
	self->visit_continue_stmt 	= va_arg(*ap, visitor_fun);
	self->visit_ret_stmt 		= va_arg(*ap, visitor_fun);

	self->visit_identifier_expr = va_arg(*ap, visitor_fun);
	self->visit_literal_expr 	= va_arg(*ap, visitor_fun);

	self->visit_unary_expr 		= va_arg(*ap, visitor_fun);
	self->visit_binary_expr 	= va_arg(*ap, visitor_fun);

	self->visit_logical_expr 	= va_arg(*ap, visitor_fun);
	self->visit_group_expr 		= va_arg(*ap, visitor_fun);

	self->visit_call_expr 					= va_arg(*ap, visitor_fun);
	self->visit_super_constructor_call_expr = va_arg(*ap, visitor_fun);

	self->visit_get_expr 		= va_arg(*ap, visitor_fun);
	self->visit_super_get_expr 	= va_arg(*ap, visitor_fun);

	self->visit_id_assign_expr 	= va_arg(*ap, visitor_fun);
	self->visit_ls_assign_expr 	= va_arg(*ap, visitor_fun);
	self->visit_set_expr 		= va_arg(*ap, visitor_fun);

	self->visit_list_expr 	= va_arg(*ap, visitor_fun);

	self->visit_list_subscript_expr = va_arg(*ap, visitor_fun);

	return _self;
}

static void *ast_visitor_destructor(void *_self) {
	return _self;
}

