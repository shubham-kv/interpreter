#if !defined(ast_object_h)
	#define ast_object_h

	#include <stdbool.h>

	void *accept_visitor(const void *ast_node, const void *ast_visitor);

	bool is_ast_node(const void *object);


	const void *get_leftmost_token(const void *self);

	const void *get_rightmost_token(const void *self);

	// Returns all the tokens constituting the node as a null terminated list .
	// void *get_tokens(const void *self);

#endif // ast_object_h
