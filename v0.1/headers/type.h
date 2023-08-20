#if !defined(type_h)
	#define type_h

	#include <stdarg.h>
	#include <stddef.h>
	#include <stdbool.h>

	struct Type {
		size_t size;

		void *(*constructor)(void *self, va_list *ap);

		void *(*destructor)(void *self);
	};

	struct AstNodeType {
		size_t size;

		void *(*constructor)(void *self, va_list *ap);

		void *(*destructor)(void *self);

		void *(*accept_visitor)(const void *self, const void *visitor);

		bool (*is_ast_node)(const void *self);

		const void *(*get_leftmost_token)(const void *self);

		const void *(*get_rightmost_token)(const void *self);

		void *(*get_tokens)(const void *self);
	};

	struct ValueBaseType {
		size_t size;

		void *(*constructor)(void *self, va_list *ap);

		void *(*destructor)(void *self);


		bool (*is_value)(const void *self);

		void (*print_value)(const void *self);

		void *(*copy_value)(const void *self);

		bool (*is_value_true)(const void *self);

		bool (*are_values_equal)(const void *self, const void *other);


		void *(*add_values)(const void *self, const void *other);

		void *(*sub_values)(const void *self, const void *other);
		
		void *(*mul_values)(const void *self, const void *other);

		void *(*div_values)(const void *self, const void *other);

		void *(*mod_values)(const void *self, const void *other);
	};

#endif // type_h
