#include "stmt_import.h"

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "type.h"
#include "object.h"
#include "ast_object.h"

#include "my_source.h"
#include "my_lexer.h"
#include "my_parser.h"

#include "ast_visitor.h"



static void *import_stmt_constructor(void *self, va_list *ap);
static void *import_stmt_destructor(void *self);

static void *import_stmt_accept_visitor(const void *self, const void *ast_visitor);
static bool is_import_stmt(const void *object);

static const void *import_stmt_get_leftmost_token(const void *self);
static const void *import_stmt_get_rightmost_token(const void *self);


static const struct AstNodeType _type_import_stmt = {
	.size 					= sizeof(struct ImportStmt),
	.constructor 			= import_stmt_constructor,
	.destructor 			= import_stmt_destructor,
	.accept_visitor 		= import_stmt_accept_visitor,
	.is_ast_node 			= is_import_stmt,
	.get_leftmost_token 	= import_stmt_get_leftmost_token,
	.get_rightmost_token 	= import_stmt_get_rightmost_token,
	.get_tokens 			= NULL
};

const void *ImportStmt = &_type_import_stmt;


// ... = new(ImportStmt, import_token, module_id_token, semicolon_token);

static void *import_stmt_constructor(void *_self, va_list *ap) {
	struct ImportStmt *self = _self;
	assert(IS_IMPORT_STMT(self));

	self->import_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->import_token));

	self->module_id = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->module_id));

	self->semicolon_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->semicolon_token));

	self->saved_source = va_arg(*ap, void *);
	assert(IS_SOURCE(self->saved_source));

	self->saved_lexer = va_arg(*ap, void *);
	assert(IS_LEXER(self->saved_lexer));

	self->saved_parser = va_arg(*ap, void *);
	assert(IS_PARSER(self->saved_parser));

	return _self;
}


static void *import_stmt_destructor(void *_self) {
	return _self;
}


static void *import_stmt_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct ImportStmt *self = _self;
	assert(IS_IMPORT_STMT(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_import_stmt);

	return ast_visitor->visit_import_stmt(_ast_visitor, _self);
}


static bool is_import_stmt(const void *object) {
	return IS_IMPORT_STMT(object);
}

static const void *import_stmt_get_leftmost_token(const void *_self) {
	const struct ImportStmt *self = _self;
	assert(IS_IMPORT_STMT(self));

	return self->import_token;
}

static const void *import_stmt_get_rightmost_token(const void *_self) {
	const struct ImportStmt *self = _self;
	assert(IS_IMPORT_STMT(self));

	return self->semicolon_token;
}


