#include "ast_object.h"

#include <stdbool.h>
#include <assert.h>

#include "type.h"


void *accept_visitor(const void *ast_node, const void *ast_visitor) {
	const struct AstNodeType *const *p = ast_node;
	assert((*p) && (*p)->accept_visitor);

	return (*p)->accept_visitor(ast_node, ast_visitor);
}

bool is_ast_node(const void *object) {
	const struct AstNodeType *const *p = object;
	assert((*p) && (*p)->is_ast_node);

	return (*p)->is_ast_node(object);
}


const void *get_leftmost_token(const void *ast_node) {
	const struct AstNodeType *const *p = ast_node;
	assert((*p) && (*p)->get_leftmost_token);

	return (*p)->get_leftmost_token(ast_node);
}

const void *get_rightmost_token(const void *ast_node) {
	const struct AstNodeType *const *p = ast_node;
	assert((*p) && (*p)->get_rightmost_token);

	return (*p)->get_rightmost_token(ast_node);
}

// void *get_tokens(const void *ast_node) {
// 	const struct AstNodeType *const *p = ast_node;
// 	assert((*p) && (*p)->get_tokens);

// 	return (*p)->get_tokens(ast_node);
// }



