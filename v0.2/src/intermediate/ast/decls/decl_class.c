#include "decl_class.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "object.h"

#include "symbol.h"
#include "symbol_table.h"
#include "ast_object.h"

#include "ast_visitor.h"


const char cur_instance_ptr_name[] = "it";
// const char super_name[] = "super";



/*
class Point {
	var x;
	var y;

	fun move(a, b) {
		it.x = a;
		it.y = b;
	}
}

pointSymbol = Symbol {
	id: 'Point'
	symbol_table: ...,
	def_type: DEF_TYPE_CLASS,
	attributes: {  }
}

ClassDecl {
	symbol: pointSymbol,
	symbol_table: {
		nesting_level: 1,
		entries: {
			it: Symbol {
				id: 'it',
				symbol_table: up,
				def_type: DEF_TYPE_IT_POINTER
			}
			x: Symbol {
				id: 'x',
				symbol_table: up,
				def_type: DEF_TYPE_MUTABLE_PROPERTY
			}
			y: Symbol {
				id: 'y',
				symbol_table: up,
				def_type: DEF_TYPE_MUTABLE_PROPERTY
			}
			move: Symbol {
				id: 'move',
				symbol_table: up,
				def_type: DEF_TYPE_METHOD
				attrs: {
					SYM_ATTR_KEY_AST: Fundecl {
						...
					}
				}
			}
		}
	}
}

*/

static void *class_decl_constructor(void *self, va_list *ap);
static void *class_decl_destructor(void *self);

static void *class_decl_accept_visitor(const void *self, const void *visitor);
bool is_class_decl(const void *self);


static const struct AstNodeType _type_class_decl = {
	.size 					= sizeof(struct ClassDecl),
	.constructor 			= class_decl_constructor,
	.destructor 			= class_decl_destructor,
	.accept_visitor 		= class_decl_accept_visitor,
	.is_ast_node 			= is_class_decl,
	.get_leftmost_token 	= NULL,
	.get_rightmost_token 	= NULL,
	.get_tokens 			= NULL
};

const void *ClassDecl = &_type_class_decl;



// ... = new(ClassDecl, class_symbol, class_symbol_table, super_class_symbol, property_count, properties, methods);

static void *class_decl_constructor(void *_self, va_list *ap) {
	struct ClassDecl *self = _self;
	assert(IS_CLASS_DECL(self));

	self->symbol = va_arg(*ap, const void *);
	assert(IS_SYMBOL(self->symbol));
	
	self->symbol_table = va_arg(*ap, void *);
	assert(IS_SYMBOL_TABLE(self->symbol_table));

	self->super_class_symbol = va_arg(*ap, void *);

	if(NULL != self->super_class_symbol) {
		assert(IS_SYMBOL(self->super_class_symbol));
	}

	self->property_count = (uint32_t) va_arg(*ap, int);
	self->properties = va_arg(*ap, void **);

	for(int i = 0; i < self->property_count; i++) {
		assert(IS_SYMBOL(self->properties[i]));
	}

	self->methods = va_arg(*ap, void *);
	// self->inner_classes = va_arg(*ap, void *);

	return _self;
}


static void *class_decl_destructor(void *_self) {
	struct ClassDecl *self = _self;
	assert(IS_CLASS_DECL(self));

	free(self->properties);

	for(int i = 0; NULL != self->methods[i]; i++)
		delete(self->methods[i]);
	
	free(self->methods);

	// for(int i = 0; NULL != self->inner_classes[i]; i++)
	// 	delete(self->inner_classes[i]);
	
	// free(self->inner_classes);

	delete(self->symbol_table);

	return _self;
}


static void *class_decl_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct ClassDecl *self = _self;
	assert(IS_CLASS_DECL(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_class_decl);

	return ast_visitor->visit_class_decl(_ast_visitor, _self);
}


bool is_class_decl(const void *_object) {
	const struct ClassDecl *object = _object;
	assert(object);

	return (object->type == ClassDecl);
}




