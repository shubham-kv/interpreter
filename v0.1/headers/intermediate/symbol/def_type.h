#if !defined(definition_type_h)
	#define definition_type_h

	// enum DefinitionType {
	// 	DEF_TYPE_VARIABLE,		// mutable variable
	// 	DEF_TYPE_CONSTANT,		// immutable variable

	// 	DEF_TYPE_FUN,			// a normal function
	// 	DEF_TYPE_NATIVE_FUN,	// predefined native funs

	// 	DEF_TYPE_CLASS,
	// 	DEF_TYPE_MUTABLE_PROPERTY,
	// 	DEF_TYPE_IMMUTABLE_PROPERTY,
	// 	DEF_TYPE_METHOD,
	// };

	enum DefinitionType {
		DEF_TYPE_VARIABLE,
		DEF_TYPE_CONSTANT,

		DEF_TYPE_FUN,
		DEF_TYPE_NATIVE_FUN,

		DEF_TYPE_CLASS,
		DEF_TYPE_MUTABLE_PROPERTY,
		DEF_TYPE_IMMUTABLE_PROPERTY,
		DEF_TYPE_CUR_INSTANCE_POINTER,
		DEF_TYPE_METHOD
	};

#endif // definition_type_h
