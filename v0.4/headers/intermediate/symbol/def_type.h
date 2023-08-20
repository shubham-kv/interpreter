#if !defined(definition_type_h)
	#define definition_type_h

	enum DefinitionType {
		DEF_TYPE_VAR,
		DEF_TYPE_VAL,

		DEF_TYPE_FUN,
		DEF_TYPE_NATIVE_FUN,

		DEF_TYPE_CLASS,

		DEF_TYPE_MUTABLE_PROPERTY,
		DEF_TYPE_IMMUTABLE_PROPERTY,

		DEF_TYPE_CUR_INSTANCE_POINTER,
		DEF_TYPE_SUPER_PTR,

		DEF_TYPE_CONSTRUCTOR,
		DEF_TYPE_METHOD
	};

#endif // definition_type_h
