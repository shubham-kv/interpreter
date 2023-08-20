// #include "stmt_print.h"

// #include <assert.h>

// #include <stdarg.h>
// #include <stdbool.h>
// #include <stddef.h>
// #include <stdlib.h>
// #include <string.h>

// #include "type.h"
// #include "object.h"
// #include "value.h"

// #include "symbol.h"
// #include "ast_object.h"

// #include "ast_visitor.h"


// static void *printstmt_constructor(void *self, va_list *ap);
// static void *printstmt_destructor(void *self);

// static void *printstmt_accept_visitor(const void *self, const void *ast_visitor);
// static bool is_print_stmt(const void *object);


// static const struct AstNodeClass _class_print_stmt = {
// 	sizeof(struct PrintStmt),
// 	printstmt_constructor,
// 	printstmt_destructor,
// 	printstmt_accept_visitor,
// 	is_print_stmt
// };

// const void *PrintStmt = &_class_print_stmt;


// // ... = new(PrintStmt, expr);

// static void *printstmt_constructor(void *_self, va_list *ap) {
// 	struct PrintStmt *self = _self;
// 	assert(self && (self->class == PrintStmt));

// 	self->expr = va_arg(*ap, const void *);
// 	assert(is_ast_node(self->expr));

// 	return _self;
// }


// static void *printstmt_destructor(void *_self) {
// 	struct PrintStmt *self = _self;
// 	assert(self && (self->class == PrintStmt));

// 	delete((void *) self->expr);
// 	return _self;
// }


// static void *printstmt_accept_visitor(const void *_self, const void *_ast_visitor) {
// 	const struct PrintStmt *self = _self;
// 	assert(self && (self->class == PrintStmt));

// 	const struct AstVisitor *ast_visitor = _ast_visitor;
// 	assert((ast_visitor->class == AstVisitor) && ast_visitor->visit_print_stmt);

// 	return ast_visitor->visit_print_stmt(_ast_visitor, _self);
// }


// static bool is_print_stmt(const void *_object) {
// 	const struct PrintStmt *object = _object;
// 	assert(object);

// 	return (object->class == PrintStmt);
// }

