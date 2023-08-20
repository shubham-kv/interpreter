#include "stmt_block.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "object.h"

#include "symbol_table.h"
#include "ast_object.h"

#include "ast_visitor.h"


static void *blockstmt_constructor(void *self, va_list *ap);
static void *blockstmt_destructor(void *self);

static void *blockstmt_accept_visitor(const void *self, const void *ast_visitor);
static bool is_block_stmt(const void *object);


static const struct AstNodeType _type_block_stmt = {
	sizeof(struct BlockStmt),
	blockstmt_constructor,
	blockstmt_destructor,
	blockstmt_accept_visitor,
	is_block_stmt
};

const void *BlockStmt = &_type_block_stmt;


// ... = new(BlockStmt, symbol_table, child_stmts);

static void *blockstmt_constructor(void *_self, va_list *ap) {
	struct BlockStmt *self = _self;
	assert(IS_BLOCK_STMT(self));

	self->symbol_table = va_arg(*ap, void *);
	assert(IS_SYMBOL_TABLE(self->symbol_table));

	self->child_stmts = va_arg(*ap, void **);
	assert(self->child_stmts);

	for(uint32_t i = 0; NULL != self->child_stmts[i]; i++) {
		assert(is_ast_node(self->child_stmts[i]));
	}

	return _self;
}


static void *blockstmt_destructor(void *_self) {
	struct BlockStmt *self = _self;
	assert(IS_BLOCK_STMT(self));

	for(uint32_t i = 0; NULL != self->child_stmts[i]; i++)
		delete(self->child_stmts[i]);
	
	free(self->child_stmts);

	delete((void *) self->symbol_table);

	return _self;
}


static void *blockstmt_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct BlockStmt *self = _self;
	assert(IS_BLOCK_STMT(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_block_stmt);

	return ast_visitor->visit_block_stmt(_ast_visitor, _self);
}


static bool is_block_stmt(const void *_object) {
	const struct BlockStmt *object = _object;
	assert(object);

	return (object->type == BlockStmt);
}

