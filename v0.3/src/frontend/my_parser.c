#include "my_parser.h"

#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#include "main.h"
#include "my_memory.h"
#include "object.h"
#include "type.h"

#include "headers_frontend.h"
#include "headers_intermediate.h"
#include "predefined.h"
#include "headers_intermediate_symbol.h"

#include "headers_backend_value.h"

#include "headers_utils.h"


#define MAX_PARSE_ERRORS 16


void* parser_constructor(void* self, va_list* ap);
void* parser_destructor(void* self);

const struct Type _class_parser = {
    .size = sizeof(struct Parser),
    .constructor = parser_constructor,
    .destructor = parser_destructor
};


const void* Parser = &_class_parser;



// ... = new(Parser, source);

void* parser_constructor(void* _self, va_list* ap) {
#if defined(DEBUG)
    vi_log("parser_constructor(self: %#x)\n", _self);
#endif // DEBUG

    struct Parser* self = _self;
    assert(IS_PARSER(self));

    self->source = va_arg(*ap, const void*);
    assert(IS_SOURCE(self->source));

    self->token_index = 0;
    self->tokens = NULL;

    self->symbol_table_stack = NULL;
    self->program_ast = NULL;

    self->err_count = 0;
    self->had_error = false;

    self->parsing_loop = false;
    self->parsing_fun = false;
    self->parsing_class = false;
    self->parsing_class_has_superclass = false;

#if defined(DEBUG)
    vi_log("ret parser_constructor(self: %#x)\n", _self);
#endif // DEBUG

    return _self;
}


void* parser_destructor(void* _self) {
	#if defined(DEBUG)
		vi_log("parser_destructor(self: %#x)\n", _self);
	#endif // DEBUG

    struct Parser* self = _self;
    assert(IS_PARSER(self));

    if (self->program_ast)
        delete(self->program_ast);

    if (self->symbol_table_stack)
        delete(self->symbol_table_stack);

	#if defined(DEBUG)
		vi_log("ret parser_destructor(self: %#x)\n", _self);
	#endif // DEBUG

    return _self;
}


#define CUR_TOKEN(parser)	((struct Token *) (parser->tokens[parser->token_index]))
#define PREV_TOKEN(parser)	((struct Token *) (parser->tokens[parser->token_index - 1]))

#define P_MATCH(parser, expected) \
		((parser->token_index < parser->tokens_count) && \
			(((const struct Token *) parser->tokens[parser->token_index])->token_type.ordinal == (expected.ordinal)))

#define P_CONSUME(parser, expected) \
		((parser->token_index < parser->tokens_count) && \
			(((const struct Token *) parser->tokens[parser->token_index])->token_type.ordinal == (expected.ordinal)) && \
			(parser->token_index++))

#define P_ADVANCE(parser) \
		((parser->token_index < parser->tokens_count) && (parser->token_index++))

#define P_PEEK_NEXT(parser, expected) \
	(((const struct Token *) parser->tokens[parser->token_index + 1])->token_type.ordinal == (expected.ordinal))


bool p_match_any(const struct Parser* self, int count, ...) {
    va_list ap;
    va_start(ap, count);

    struct Token* token = CUR_TOKEN(self);

    for (uint32_t i = 0; i < count; i++) {
        const struct TokenType expected = va_arg(ap, const struct TokenType);

        if (token->token_type.ordinal == expected.ordinal) {
            va_end(ap);
            return true;
        }
    }

    va_end(ap);
    return false;
}

/*
void p_advance(struct Parser* self) {
    struct Token* token = CUR_TOKEN(self);
    if (token->token_type.ordinal != TOKEN_TYPE_EOF.ordinal) {
        self->token_index++;
    }
}

bool p_consume(struct Parser* self, struct TokenType expected) {
    if (P_MATCH(self, expected)) {
        P_ADVANCE(self);
        return true;
    }
    return false;
}
// */




void p_flag_error_va(
    struct Parser* self,
    bool display_line, const uint32_t line_number,
    const char* err_line, const uint32_t err_index, const uint32_t err_token_length,
    const char* err_msg, va_list* ap
) {

    // set color to red
	#if defined(_WIN32)
		HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
		GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);

		WORD savedAttr = consoleInfo.wAttributes;

		SetConsoleTextAttribute(consoleHandle, FOREGROUND_RED);
	#else
		fprintf(stderr, COLOR_RED);
	#endif

    fprintf(stderr, "\n");
    vfprintf(stderr, err_msg, *ap);
    fprintf(stderr, "\n");

    if (display_line) {
        fprintf(stderr, "-%03d-| ", line_number);

        for (uint32_t i = 0; err_line[i] != '\0'; i++) {
            char c = err_line[i];

            if ('\t' == c)
                c = ' ';

            fprintf(stderr, "%c", c);
        }

        for (uint32_t i = 0; i < (err_index + 7); i++)
            fprintf(stderr, " ");

        for (uint32_t i = 0; i < err_token_length; i++)
            fprintf(stderr, "^");

        fprintf(stderr, "\n");
    }

    // restore the original color
	#if defined(_WIN32)
		SetConsoleTextAttribute(consoleHandle, savedAttr);
	#else
		fprintf(stderr, COLOR_RESET);
	#endif

    if (is_prompt_mode()) return;

    if (self->err_count >= MAX_PARSE_ERRORS) {
        fprintf(stderr, "\nToo many errors!\n");
        exit(EXIT_FAILURE);
    }

    self->err_count++;
}


void p_raise_error(
    struct Parser* self,
    const struct Token* err_token,
    const char* err_msg, ...
) {

    if (self->had_error) {
        return;
    }

    self->had_error = true;

    va_list ap;
    va_start(ap, err_msg);

    if (NULL == err_token) {
        p_flag_error_va(self, false, 0, NULL, 0, 0, err_msg, &ap);
    }
    else {
        const int line_index = err_token->line_index;
        const char* err_line = SOURCE_GET_LINE(self->source, line_index);
        const int err_index = err_token->start_index;

        p_flag_error_va(
            self, true, (line_index + 1),
            err_line, err_index, err_token->length, err_msg, &ap
        );
    }

    va_end(ap);
}




void p_raise_error_expr(
    struct Parser* self,
    const void* expr,
    const char* err_msg, ...
) {

    if (self->had_error) {
        return;
    }

    self->had_error = true;

    va_list ap;
    va_start(ap, err_msg);

    const struct Token* lmost = get_leftmost_token(expr);
    const struct Token* rmost = get_rightmost_token(expr);

    // set color to red
	#if defined(_WIN32)
		HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
		GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);
		WORD savedAttr = consoleInfo.wAttributes;
		SetConsoleTextAttribute(consoleHandle, FOREGROUND_RED);
	#else
		fprintf(stderr, COLOR_RED);
	#endif

    fprintf(stderr, "\n");
    vfprintf(stderr, err_msg, ap);
    fprintf(stderr, "\n");

    if (lmost->line_index == rmost->line_index) {
        const int line_number = lmost->line_index + 1;
        fprintf(stderr, "-%03d-| ", line_number);

        const char* err_line = SOURCE_GET_LINE(self->source, lmost->line_index);
        const int start = lmost->start_index;
        const int end = rmost->start_index + rmost->length;

        for (int i = 0; err_line[i] != '\0'; i++) {
            char c = err_line[i];

            if ('\t' == c) c = ' ';

            fprintf(stderr, "%c", c);
        }

        for (int i = 0; i < (start + 7); i++)
            fprintf(stderr, " ");

        for (int i = 0; i < (end - start); i++)
            fprintf(stderr, "^");

        fprintf(stderr, "\n");
    }

    // what if they are on diff. lines
    else {

    }

    // restore the original color
	#if defined(_WIN32)
		SetConsoleTextAttribute(consoleHandle, savedAttr);
	#else
		fprintf(stderr, COLOR_RESET);
	#endif


    va_end(ap);

    if (is_prompt_mode())
        return;

    if (self->err_count >= MAX_PARSE_ERRORS) {
        fprintf(stderr, "\nParser: Too many errors!\n");
        exit(EXIT_FAILURE);
    }

    self->err_count++;
}




/*
|==============================>
| Declaration parser methods.
|==============================>
*/
void* program(struct Parser* self);

void* import_stmt(struct Parser* self);

void* declaration(struct Parser* self);

void* var_decl(
    struct Parser* self,
    enum DefinitionType def_type, bool init_required
);

void* var_init(
    struct Parser* self, const struct Token* prev_token,
    enum DefinitionType def_type, bool init_required
);


void* val_decl(
    struct Parser* self,
    enum DefinitionType def_type, bool init_required
);

void* val_init(
    struct Parser* self, const struct Token* prev_token,
    enum DefinitionType def_type, bool init_required
);


void* fun_decl(struct Parser* self);
void* function(struct Parser* self, enum DefinitionType def_type);

void* class_decl(struct Parser* self);

/*
|==============================>
| Statement parser methods.
|==============================>
*/
void* stmt(struct Parser* self);

void* expr_stmt(struct Parser* self);
// void *print_stmt(struct Parser *self);

void* if_stmt(struct Parser* self);
void* when_stmt(struct Parser* self);

void* loop_stmt(struct Parser* self);
void* repeat_stmt(struct Parser* self);

void* block_stmt(struct Parser* self);

void* break_stmt(struct Parser* self);
void* continue_stmt(struct Parser* self);
void* ret_stmt(struct Parser* self);


/*
|==============================>
| Expression parser methods.
|==============================>
*/
void* expr(struct Parser* self);

void* assignment(struct Parser* self);

void* logical_or(struct Parser* self);
void* logical_and(struct Parser* self);

void* equality(struct Parser* self);
void* comparison(struct Parser* self);

void* term(struct Parser* self);
void* factor(struct Parser* self);

void* unary(struct Parser* self);
void* molecule(struct Parser* self);
void* atom(struct Parser* self);






// program -> declaration* EOF ;

void* program(struct Parser* self) {
	#if defined(DEBUG)
		vi_log("program()\n");
	#endif // DEBUG

    bool is_last_ast_null = false;

    uint64_t decls_count = 0;
    void** declarations = NULL;

    uint64_t imports_count = 0;
    void** imports = NULL;

    void* program_symbol_table = (void*) symtabstack_push(self->symbol_table_stack);

    for (
        const struct Token* token = CUR_TOKEN(self);
        (token->token_type.ordinal != TOKEN_TYPE_EOF.ordinal) && (token->token_type.ordinal != TOKEN_TYPE_ERROR.ordinal);
        token = CUR_TOKEN(self)
    ) {

        if (P_MATCH(self, TOKEN_TYPE_IMPORT)) {
            void* importstmt = import_stmt(self);

			if(NULL == import_stmt)
				break;

			// else if(NULL != import_stmt) {
			imports = reallocate(imports, sizeof(void*), ++imports_count);
			imports[imports_count - 1] = importstmt;

			void* import_parser = IMPORT_STMT_PARSER(importstmt);
			void* prog_ast = PARSER_PROGRAM_AST(import_parser);
			symbol_table_copy(program_symbol_table, PROGRAM_AST_SYMBOL_TABLE(prog_ast));
            // }
        }
        else {
            void* decl = declaration(self);

            if (NULL == decl)
                break;

			// else if(NULL != decl) {
            if (self->is_import) {
                if (!IS_CLASS_DECL(decl) && !IS_FUN_DECL(decl) && !IS_VAR_DECL(decl)) {
                    delete(decl);
                    continue;
                }
            }

            if (NULL != decl) {
                declarations = reallocate(declarations, sizeof(void*), ++decls_count);
                declarations[decls_count - 1] = decl;
            }
			// }
        }
    }

    symtabstack_pop(self->symbol_table_stack);	// program_symbol_table

    const char* source_name = SOURCE_INSTREAM(self->source);
    size_t len = strlen(source_name);

    char* program_name = allocate(sizeof(char), len + 1);
    strncpy(program_name, source_name, len);

    void* program_ast = new(
        ProgramAst, program_name, program_symbol_table,
        imports_count, imports,
        decls_count, declarations
	);

	#if defined(DEBUG)
		vi_log("ret program()\n");
	#endif // DEBUG

    return program_ast;
}


void* import_stmt(struct Parser* self) {
	#if defined(DEBUG)
		vi_log("import_stmt()\n");
	#endif // DEBUG

    if (!P_MATCH(self, TOKEN_TYPE_IMPORT)) {
        p_raise_error(
            self, CUR_TOKEN(self),
            "SyntaxError: Expected 'import' found '%.*s'.",
            CUR_TOKEN(self)->length, CUR_TOKEN(self)->start
        );
        return NULL;
    }

    const struct Token* import_token = CUR_TOKEN(self);
    P_ADVANCE(self);

    const struct Token* module_id = NULL;

    if (!P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
        p_raise_error(
            self, import_token,
            "SyntaxError: Expected module's name after 'import'."
        );
        return NULL;
    }
    else {
        module_id = CUR_TOKEN(self);
        P_ADVANCE(self);
    }

    const struct Token* semicolon = NULL;

    if (!P_MATCH(self, TOKEN_TYPE_SEMICOLON)) {
        p_raise_error(
            self, module_id,
            "SyntaxError: Expected ';' after module's name."
        );
        return NULL;
    }
    else {
        semicolon = CUR_TOKEN(self);
        P_ADVANCE(self);
    }

    const char* instream = SOURCE_INSTREAM(self->source);


    size_t ext_len = strlen(extension);
    size_t len = module_id->length + ext_len;

    char* module_name = allocate(sizeof(char), len + 1);
    strncpy(module_name, module_id->start, module_id->length);
    strncpy(module_name + module_id->length, extension, ext_len);

    if (strncmp(instream, module_name, len) == 0) {
        p_raise_error(
            self, module_id,
            "ParseError: Invalid import of current module '%s'.",
            module_name
        );
        free(module_name);
        return NULL;
    }

    void* source = new(Source, module_name);
    source_read_file(source, module_name);

    void* lexer = new(Lexer, source);
    scan(lexer);

    if (LEXER_HAD_ERROR(lexer)) {
        delete(source);
        delete(lexer);
        return NULL;
    }

    void* parser = new(Parser, source);
    parse(parser, true, lexer);

    if (PARSER_HAD_ERROR(parser)) {
        delete(source);
        delete(lexer);
        delete(parser);
        return NULL;
    }

    void* importstmt = new(
        ImportStmt, import_token, module_id, semicolon,
        source, lexer, parser
	);

	#if defined(DEBUG)
		vi_log("ret import_stmt()\n");
	#endif // DEBUG

    return importstmt;
}


void* declaration(struct Parser* self) {
    if (P_MATCH(self, TOKEN_TYPE_IMPORT))
        return import_stmt(self);

    if (P_MATCH(self, TOKEN_TYPE_VAR))
        return var_decl(self, DEF_TYPE_VAR, true);

    else if (P_MATCH(self, TOKEN_TYPE_VAL))
        return val_decl(self, DEF_TYPE_VAL, true);

    else if (P_MATCH(self, TOKEN_TYPE_FUN))
        return fun_decl(self);

    else if (P_MATCH(self, TOKEN_TYPE_CLASS))
        return class_decl(self);

    else
        return stmt(self);
}




// var_decl -> "var" var_init ("," var_init)* ";" ;
// var_init -> IDENTIFIER ("=" expr)?

void* var_decl(
    struct Parser* self,
    enum DefinitionType def_type,
    bool init_required
) {

#if defined(DEBUG)
    vi_log("var_decl()\n");
#endif // DEBUG

    if (!P_MATCH(self, TOKEN_TYPE_VAR)) {
        p_raise_error(
            self, CUR_TOKEN(self),
            "SyntaxError: Expected 'var' found '%.*s'.",
            CUR_TOKEN(self)->length, CUR_TOKEN(self)->start
        );
        return NULL;
    }

    const struct Token* var_token = CUR_TOKEN(self);
    P_ADVANCE(self);

    void* symbol = var_init(self, var_token, def_type, init_required);

    if (NULL == symbol)
        return NULL;


    uint32_t symbols_count = 0;

    void** symbols = allocate(sizeof(void*), ++symbols_count);
    symbols[symbols_count - 1] = symbol;

    const struct Token* comma_token = NULL;

    while (P_MATCH(self, TOKEN_TYPE_COMMA)) {
        comma_token = CUR_TOKEN(self);
        P_ADVANCE(self);

        symbol = var_init(self, comma_token, def_type, init_required);

        if (NULL == symbol) {
            free(symbols);
            return NULL;
        }

        symbols = reallocate(symbols, sizeof(void*), ++symbols_count);
        symbols[symbols_count - 1] = symbol;
    }

    if (!P_CONSUME(self, TOKEN_TYPE_SEMICOLON)) {
        free(symbols);

        p_raise_error(
            self, var_token,
            "SyntaxError: Expected ';' after var statement."
        );

        return NULL;
    }

    symbols = reallocate(symbols, sizeof(void*), ++symbols_count);
    symbols[symbols_count - 1] = NULL;

#if defined(DEBUG)
    vi_log("ret var_decl()\n");
#endif // DEBUG

    return new(VarDecl, symbols);
}


// Grammar
// var_init -> IDENTIFIER ("=" expr)? ;

void* var_init(
    struct Parser* self, const struct Token* prev_token,
    enum DefinitionType def_type, bool init_required
) {

#if defined(DEBUG)
    vi_log("var_init()\n");
#endif // DEBUG

    const struct Token* id_token = NULL;

    if (P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
        id_token = CUR_TOKEN(self);
        P_ADVANCE(self);
    }
    else {
        p_raise_error(
            self, prev_token,
            "SyntaxError: Expected Identifier after '%.*s'.",
            prev_token->length, prev_token->start
        );

        return NULL;
    }

    char* id = allocate(sizeof(char), id_token->length + 1);
    strncpy(id, id_token->start, id_token->length);

    const void* local_symbol_table = symtabstack_get_local_table(self->symbol_table_stack);

    if (NULL != symbol_table_lookup(local_symbol_table, id)) {
        free(id);

        p_raise_error(
            self, id_token,
            "Error: Redefinition of '%s'.", id
        );

        return NULL;
    }


    void* init_expr = NULL;

    if (init_required) {
        if (!P_MATCH(self, TOKEN_TYPE_EQUAL)) {
            free(id);

            p_raise_error(
                self, id_token,
                "Error: Expected some initializer expression."
            );
            return NULL;
        }
        else {
            const struct Token* equal_token = CUR_TOKEN(self);
            P_ADVANCE(self);

            init_expr = expr(self);

            if (NULL == init_expr) {
                free(id);

                p_raise_error(
                    self, equal_token,
                    "SyntaxError: Expected some expression after '='."
                );
                return NULL;
            }
        }
    }

    void** symbol_attributes = allocate(sizeof(void*), TOTAL_NO_OF_KEYS);

    void* symbol = new(Symbol, id, local_symbol_table, def_type, symbol_attributes);

    if (NULL != init_expr) {
        symbol_set_attr(symbol, KEY_INIT_EXPR, init_expr);
    }

    symbol_table_define(local_symbol_table, id, symbol);

#if defined(DEBUG)
    vi_log("ret var_init()\n");
#endif // DEBUG

    return symbol;
}



// Grammar
// val_decl -> "val" val_init ("," val_init)* ";" ;
// val_init -> IDENTIFIER ("=" expr)? ;

void* val_decl(
    struct Parser* self,
    enum DefinitionType def_type, bool init_required
) {

#if defined(DEBUG)
    vi_log("val_decl()\n");
#endif // DEBUG

    if (!P_MATCH(self, TOKEN_TYPE_VAL)) {
        p_raise_error(
            self, CUR_TOKEN(self),
            "SyntaxError: Expected 'val' found '%.*s'.",
            CUR_TOKEN(self)->length, CUR_TOKEN(self)->start
        );
        return NULL;
    }

    const struct Token* val_token = CUR_TOKEN(self);
    P_ADVANCE(self);

    void* symbol = val_init(self, val_token, def_type, init_required);

    if (NULL == symbol)
        return NULL;

    uint32_t symbols_count = 0;

    void** symbols = allocate(sizeof(void*), ++symbols_count);
    symbols[symbols_count - 1] = symbol;

    const struct Token* comma_token = NULL;

    while (P_MATCH(self, TOKEN_TYPE_COMMA)) {
        comma_token = CUR_TOKEN(self);
        P_ADVANCE(self);

        symbol = val_init(self, comma_token, def_type, init_required);

        if (NULL == symbol) {
            free(symbols);
            return NULL;
        }

        symbols = reallocate(symbols, sizeof(void*), ++symbols_count);
        symbols[symbols_count - 1] = symbol;
    }

    if (!P_CONSUME(self, TOKEN_TYPE_SEMICOLON)) {
        free(symbols);

        p_raise_error(
            self, val_token,
            "SyntaxError: Expected ';' after val statement."
        );

        return NULL;
    }

    symbols = reallocate(symbols, sizeof(void*), ++symbols_count);
    symbols[symbols_count - 1] = NULL;

#if defined(DEBUG)
    vi_log("ret var_decl()\n");
#endif // DEBUG

    return new(VarDecl, symbols);
}


// Grammar
// val_init -> IDENTIFIER "=" expr ;

void* val_init(
    struct Parser* self, const struct Token* prev_token,
    enum DefinitionType def_type, bool init_required
) {

#if defined(DEBUG)
    vi_log("val_init()\n");
#endif // DEBUG

    const struct Token* id_token = NULL;

    if (P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
        id_token = CUR_TOKEN(self);
        P_ADVANCE(self);
    }
    else {
        p_raise_error(
            self, prev_token,
            "SyntaxError: Expected Identifier after '%.*s'.", prev_token->length, prev_token->start
        );

        return NULL;
    }


    char* id = allocate(sizeof(char), id_token->length + 1);
    strncpy(id, id_token->start, id_token->length);

    const void* local_symbol_table = symtabstack_get_local_table(self->symbol_table_stack);

    // if(NULL != symtabstack_lookup(self->symbol_table_stack, id)) {
    if (NULL != symbol_table_lookup(local_symbol_table, id)) {
        p_raise_error(
            self, id_token,
            "Error: Redefinition of '%s'.", id
        );
        free(id);
        return NULL;
    }

    void* init_expr = NULL;

    if (init_required) {
        if (!P_MATCH(self, TOKEN_TYPE_EQUAL)) {
            free(id);

            p_raise_error(
                self, id_token,
                "Error: Expected some initializer expression."
            );
            return NULL;
        }
        else {
            const struct Token* equal_token = CUR_TOKEN(self);
            P_ADVANCE(self);

            init_expr = expr(self);

            if (NULL == init_expr) {
                free(id);

                p_raise_error(
                    self, equal_token,
                    "SyntaxError: Expected some expression after '='."
                );
                return NULL;
            }
        }
    }

    void** symbol_attributes = allocate(sizeof(void*), TOTAL_NO_OF_KEYS);

    void* symbol = new(Symbol, id, local_symbol_table, def_type, symbol_attributes);

    if (NULL != init_expr) {
        symbol_set_attr(symbol, KEY_INIT_EXPR, init_expr);
    }

    symbol_table_define(local_symbol_table, id, symbol);

#if defined(DEBUG)
    vi_log("ret val_init()\n");
#endif // DEBUG

    return symbol;
}


// Grammar
// fun_decl ->  "fun" function ;

void* fun_decl(struct Parser* self) {
#if defined(DEBUG)
    vi_log("fun_decl()\n");
#endif // DEBUG

    if (!P_MATCH(self, TOKEN_TYPE_FUN)) {

        if (!P_MATCH(self, TOKEN_TYPE_EOF)) {
            p_raise_error(
                self, CUR_TOKEN(self),
                "SyntaxError: Expected 'fun', found '%.*s'.",
                CUR_TOKEN(self)->length, CUR_TOKEN(self)->start
            );
        }

        return NULL;
    }

    const struct Token* fun_token = CUR_TOKEN(self);
    P_ADVANCE(self);

    void* fun_declaration = function(self, DEF_TYPE_FUN);

#if defined(DEBUG)
    vi_log("ret fun_decl()\n");
#endif // DEBUG

    return fun_declaration;
}


// Grammar
// function		->  IDENTIFIER '(' parameters? ')' '{' declaration* '}' ;
// parameters	->  IDENTIFIER (',' IDENTIFIER) ;

void* function(
    struct Parser* self,
    enum DefinitionType def_type
) {

#if defined(DEBUG)
    vi_log("function(def_type: %d)\n", def_type);
#endif // DEBUG

    const struct Token* fun_id_token = NULL;

    if (P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
        fun_id_token = CUR_TOKEN(self);
        P_ADVANCE(self);
    }
    else {

        if (DEF_TYPE_FUN == def_type) {
            p_raise_error(
                self, PREV_TOKEN(self),
                "SyntaxError: Expected function's name after '%.*s'",
                PREV_TOKEN(self)->length, PREV_TOKEN(self)->start
            );
        }
        else {
            p_raise_error(
                self, PREV_TOKEN(self),
                "SyntaxError: Expected some method after '%.*s'",
                PREV_TOKEN(self)->length, PREV_TOKEN(self)->start
            );
        }

        return NULL;
    }

    char* fun_id = allocate(sizeof(char), fun_id_token->length + 1);
    strncpy(fun_id, fun_id_token->start, fun_id_token->length);


    // check if somtheing is already defined with that name
    const void* id_symbol = symtabstack_lookup(self->symbol_table_stack, fun_id);

    if (NULL != id_symbol) {
        if (DEF_TYPE_VAR == SYMBOL_GET_DEF_TYPE(id_symbol)) {
            p_raise_error(
                self, fun_id_token,
                "Error: '%s' already declared as a varible.", fun_id
            );
        }
        else if (DEF_TYPE_VAL == SYMBOL_GET_DEF_TYPE(id_symbol)) {
            p_raise_error(
                self, fun_id_token,
                "Error: '%s' already declared as a val constant.", fun_id
            );
        }
        else if (DEF_TYPE_FUN == SYMBOL_GET_DEF_TYPE(id_symbol)) {
            p_raise_error(
                self, fun_id_token,
                "Error: Redefinition of function '%s'.", fun_id
            );
        }
        else {
            p_raise_error(
                self, fun_id_token,
                "Error: Redefinition of '%s'.", fun_id
            );
        }
        free(fun_id);

        return NULL;
    }


    const struct Token* lparen_token = NULL;

    if (P_MATCH(self, TOKEN_TYPE_LPAREN)) {
        lparen_token = CUR_TOKEN(self);
        P_ADVANCE(self);
    }
    else {
        free(fun_id);

        p_raise_error(
            self, fun_id_token,
            "SyntaxError: Expected '(' after '%.*s'", fun_id_token->length, fun_id_token->start
        );

        return NULL;
    }

    const void* parent_symbol_table = symtabstack_get_local_table(self->symbol_table_stack);
    const void* fun_symbol_table = symtabstack_push(self->symbol_table_stack);

    int param_count = 0;
    void** param_symbols = NULL;


    // parse the parameters
    if (P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
        const struct Token* param_token = CUR_TOKEN(self);
        P_ADVANCE(self);



        char* param_id = allocate(sizeof(char), param_token->length + 1);
        strncpy(param_id, param_token->start, param_token->length);

        void* param_symbol = new(Symbol, param_id, fun_symbol_table, DEF_TYPE_VAR, NULL);
        symbol_table_define(fun_symbol_table, param_id, param_symbol);



        param_symbols = allocate(sizeof(void*), ++param_count);
        param_symbols[param_count - 1] = param_symbol;

        while (P_MATCH(self, TOKEN_TYPE_COMMA)) {
            const struct Token* comma_token = CUR_TOKEN(self);
            P_ADVANCE(self);

            if (!P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
                delete((void*)symtabstack_pop(self->symbol_table_stack));

                p_raise_error(
                    self, comma_token,
                    "SyntaxError: Expected some parameter after ','."
                );

                return NULL;
            }

            param_token = CUR_TOKEN(self);
            P_ADVANCE(self);

            if (param_count >= MAX_PARAM_COUNT) {
                p_raise_error(
                    self, param_token,
                    "Error: Cannot have more than %d parameters.", MAX_PARAM_COUNT
                );

                return NULL;
            }

            param_id = allocate(sizeof(char), param_token->length + 1);
            strncpy(param_id, param_token->start, param_token->length);

            if (NULL != symbol_table_lookup(fun_symbol_table, param_id)) {
                delete((void*)symtabstack_pop(self->symbol_table_stack));

                p_raise_error(
                    self, param_token,
                    "Error: Parameter redefinition of '%s'.", param_id
                );

                return NULL;
            }

            param_symbol = new(Symbol, param_id, fun_symbol_table, DEF_TYPE_VAR, NULL);
            symbol_table_define(fun_symbol_table, param_id, param_symbol);

            param_symbols = reallocate(param_symbols, sizeof(void *), ++param_count);
            param_symbols[param_count - 1] = param_symbol;
        }
    }

    const struct Token* rparen_token = NULL;

    if (P_MATCH(self, TOKEN_TYPE_RPAREN)) {
        rparen_token = CUR_TOKEN(self);
        P_ADVANCE(self);
    }
    else {
        delete((void*)symtabstack_pop(self->symbol_table_stack));
        free(param_symbols);

        p_raise_error(
            self, lparen_token,
            "SyntaxError: Unmatched '(', expected corresponding ')'."
        );
        return NULL;
    }

    // define the fun's symbol to allow recursion
    const void* fun_symbol = new(Symbol, fun_id, parent_symbol_table, def_type, NULL);
    symbol_table_define(parent_symbol_table, fun_id, fun_symbol);

    const struct Token* lbrace_token = NULL;

    self->parsing_fun = true;

    if (P_MATCH(self, TOKEN_TYPE_LCURLY_BRACE)) {
        lbrace_token = CUR_TOKEN(self);
        P_ADVANCE(self);
    }
    else {
        delete((void*)symtabstack_pop(self->symbol_table_stack));
        free(param_symbols);

        p_raise_error(
            self, rparen_token,
            "SyntaxError: Expected '{' after ')'."
        );
        return NULL;
    }

    int decl_count = 0;
    void** declarations = NULL;

    #define FUN_FREE(self, decls, count) \
		do { \
			for(int i = 0; i < count; i++) \
				delete(decls[i]); \
			free(decls); 		\
			free(param_symbols);\
			\
			delete((void *) symtabstack_pop(self->symbol_table_stack)); \
		} while(0)


    while (!p_match_any(self, 2, TOKEN_TYPE_RCURLY_BRACE, TOKEN_TYPE_EOF)) {
        void* decl = declaration(self);

        if (NULL == decl) {
            FUN_FREE(self, declarations, decl_count);
            return NULL;
        }

        declarations = reallocate(declarations, sizeof(void*), ++decl_count);
        declarations[decl_count - 1] = decl;
    }

    if (!P_CONSUME(self, TOKEN_TYPE_RCURLY_BRACE)) {
        FUN_FREE(self, declarations, decl_count);

        p_raise_error(
            self, lbrace_token,
            "SyntaxError: Unmatched '{', expected corresponding '}'."
        );

        return NULL;
    }

    declarations = reallocate(declarations, sizeof(void*), ++decl_count);
    declarations[decl_count - 1] = NULL;

    symtabstack_pop(self->symbol_table_stack);	// pop fun's symbol_table


    void* fun_declaration = new(
        FunDecl, fun_symbol, fun_symbol_table,
        param_count, param_symbols, declarations
    );

    self->parsing_fun = false;

	#if defined(DEBUG)
		vi_log("ret function(def_type: %d)\n", def_type);
	#endif // DEBUG

    return fun_declaration;
}



// Grammar
//	class_decl	->  "class" IDENTIFIER ( ':' IDENTIFIER )? "{" function* "} ;

void* class_decl(struct Parser* self) {

	#if defined(DEBUG)
		vi_log("class_decl()\n");
	#endif // DEBUG

    const struct Token* class_token = CUR_TOKEN(self);
    P_ADVANCE(self);

    const struct Token* class_id_token = NULL;

    if (P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
        class_id_token = CUR_TOKEN(self);
        P_ADVANCE(self);
    }
    else {
        p_raise_error(
            self, class_token,
            "SyntaxError: Expected class's name."
        );

        return NULL;
    }

    const struct Token* colon_token = NULL;
    const struct Token* super_class_id_token = NULL;
    const void* super_class_symbol = NULL;

    if (P_MATCH(self, TOKEN_TYPE_COLON)) {
        colon_token = CUR_TOKEN(self);
        P_ADVANCE(self);

        if (P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
            super_class_id_token = CUR_TOKEN(self);
            P_ADVANCE(self);

            if (
                (class_id_token->length == super_class_id_token->length) &&
                (strncmp(class_id_token->start, super_class_id_token->start, class_id_token->length) == 0)
                ) {

                p_raise_error(
                    self, super_class_id_token,
                    "Error: Super class cannot be the same as subclass."
                );

                return NULL;
            }

            char* super_class_id = allocate(sizeof(char), super_class_id_token->length + 1);
            strncpy(super_class_id, super_class_id_token->start, super_class_id_token->length);

            super_class_symbol = symtabstack_lookup(self->symbol_table_stack, super_class_id);
            free(super_class_id);

            if (NULL == super_class_symbol) {
                p_raise_error(
                    self, super_class_id_token,
                    "Error: '%.*s' is undefined.", super_class_id_token->length, super_class_id_token->start
                );

                return NULL;
            }

            if (DEF_TYPE_CLASS != SYMBOL_GET_DEF_TYPE(super_class_symbol)) {
                p_raise_error(
                    self, super_class_id_token,
                    "Error: '%.*s' is not a class.", super_class_id_token->length, super_class_id_token->start
                );

                return NULL;
            }

            self->parsing_class_has_superclass = true;
        }
        else {
            p_raise_error(
                self, colon_token,
                "SyntaxError: Expected super class's name after ':'."
            );

            return NULL;
        }
    }


    const struct Token* lbrace_token = NULL;

    if (P_MATCH(self, TOKEN_TYPE_LCURLY_BRACE)) {
        lbrace_token = CUR_TOKEN(self);
        P_ADVANCE(self);
    }
    else {
        p_raise_error(
            self, class_token,
            "SyntaxError: Expected class' body."
        );

        return NULL;
    }

    const void* parent_symbol_table = symtabstack_get_local_table(self->symbol_table_stack);

    // parsing class' body
    void* class_symbol_table = (void*)symtabstack_push(self->symbol_table_stack);


    // define this pointer
    size_t len = strlen(cur_instance_ptr_name);

    char* instance_ptr = allocate(sizeof(char), len + 1);
    strncpy(instance_ptr, cur_instance_ptr_name, len);

    void* instance_ptr_symbol = new(Symbol, instance_ptr, class_symbol_table, DEF_TYPE_CUR_INSTANCE_POINTER, NULL);
    symbol_table_define(class_symbol_table, instance_ptr, instance_ptr_symbol);


    if (NULL != super_class_symbol) {
        len = strlen(super_name);

        char* super_ptr = allocate(sizeof(char), len + 1);
        strncpy(super_ptr, super_name, len);

        void* super_ptr_symbol = new(Symbol, super_ptr, class_symbol_table, DEF_TYPE_SUPER_PTR, NULL);
        symbol_table_define(class_symbol_table, super_ptr, super_ptr_symbol);
    }


    self->parsing_class = true;

    // uint32_t property_count = 0;
    // void **properties = NULL;

    int method_count = 0;
    void** methods = NULL;

    // int inner_class_count = 0;
    // void **inner_classes = NULL;

    void* constr_decl = NULL;

    #define CLASS_DECL_FREE(self, constr_decl, methods)	\
		do { \
			if(NULL != constr_decl) {	\
				delete(constr_decl);	\
			}	\
			if(methods) { \
				for(int i = 0; NULL != methods[i]; i++)	{	\
					delete((void *) methods[i]);	\
				}	\
				free(methods);	\
			}	\
			delete((void *) symtabstack_pop(self->symbol_table_stack));	\
		} while(0)


    while (!p_match_any(self, 2, TOKEN_TYPE_RCURLY_BRACE, TOKEN_TYPE_EOF)) {

        /*
        if(
            P_MATCH(self, TOKEN_TYPE_VAR) ||
            P_MATCH(self, TOKEN_TYPE_VAL)
        ) {

            const struct VarDecl *vardecl = NULL;

            if(P_MATCH(self, TOKEN_TYPE_VAR)) {
                vardecl = var_decl(self, DEF_TYPE_MUTABLE_PROPERTY, false);
            }
            else if(P_MATCH(self, TOKEN_TYPE_VAL)) {
                vardecl = val_decl(self, DEF_TYPE_IMMUTABLE_PROPERTY, false);
            }

            int last_count = property_count;
            int count = 0;

            for(int i = 0; NULL != vardecl->symbols[i]; i++) {
                count++;
            }

            property_count += count;
            properties = reallocate(properties, sizeof(void *), property_count);

            for(int i = last_count, j = 0; i < property_count; i++, j++) {
                properties[i] = vardecl->symbols[j];
            }

            delete((void *) vardecl);
        }

        // support inner classes
        else if(P_MATCH(self, TOKEN_TYPE_CLASS)) {
            struct ClassDecl *classdecl = (struct ClassDecl *) class_decl(self);
            inner_classes = reallocate(inner_classes, sizeof(void *), ++inner_class_count);
            inner_classes[inner_class_count - 1] = classdecl;
        }

        else
        */

        if (P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
            struct FunDecl* method = (struct FunDecl*)function(self, DEF_TYPE_METHOD);

            if (NULL == method) {
                CLASS_DECL_FREE(self, constr_decl, methods);
                return NULL;
            }

            const char* method_id = SYMBOL_GET_ID(method->symbol);
            size_t len = strlen(constructor_name);

            // check if the method is a constructor
            if (
                (strlen(method_id) == len) &&
                (strncmp(constructor_name, method_id, len) == 0)
			) {
                struct Symbol* method_symbol = (struct Symbol*)method->symbol;
                method_symbol->definition_type = DEF_TYPE_CONSTRUCTOR;

                constr_decl = method;

                continue;
            }

            // TODO: want to support method polymorphism
            methods = reallocate(methods, sizeof(void*), ++method_count);
            methods[method_count - 1] = method;
        }

        else {
            // free and return
            CLASS_DECL_FREE(self, constr_decl, methods);

            p_raise_error(
                self, CUR_TOKEN(self),
                "SemanticError: Illegal code within class's context."
            );
            return NULL;
        }
    }

    const struct Token* rbrace_token = NULL;

    if (!P_CONSUME(self, TOKEN_TYPE_RCURLY_BRACE)) {
        CLASS_DECL_FREE(self, constr_decl, methods);

        p_raise_error(
            self, lbrace_token,
            "SyntaxError: Unmatched '{', expected corressponding '}'."
        );

        return NULL;
    }

    self->parsing_class = false;
    self->parsing_class_has_superclass = false;

    // inner_classes = reallocate(inner_classes, sizeof(void *), ++inner_class_count);
    // inner_classes[inner_class_count - 1] = NULL;

    methods = reallocate(methods, sizeof(void*), ++method_count);
    methods[method_count - 1] = NULL;

    symtabstack_pop(self->symbol_table_stack);	// class_symbol_table


    char* class_id = allocate(sizeof(char), class_id_token->length + 1);
    strncpy(class_id, class_id_token->start, class_id_token->length);

    void* class_symbol = new(Symbol, class_id, parent_symbol_table, DEF_TYPE_CLASS, NULL);
    symbol_table_define(parent_symbol_table, class_id, class_symbol);

    void* classdecl = new(
        ClassDecl, class_symbol, class_symbol_table,
        super_class_symbol, constr_decl, methods
        );

#if defined(DEBUG)
    vi_log("ret class_decl(): classdecl at %#x\n", classdecl);
#endif // DEBUG

    return classdecl;
}








// Grammar
// stmt -> expr_stmt | if_stmt | when_stmt | loop_stmt | repeat_stmt | block_stmt

void* stmt(struct Parser* self) {
    if (P_MATCH(self, TOKEN_TYPE_IF))
        return if_stmt(self);

    if (P_MATCH(self, TOKEN_TYPE_WHEN))
        return when_stmt(self);

    else if (P_MATCH(self, TOKEN_TYPE_LOOP))
        return loop_stmt(self);

    else if (P_MATCH(self, TOKEN_TYPE_REPEAT))
        return repeat_stmt(self);

    else if (P_MATCH(self, TOKEN_TYPE_LCURLY_BRACE))
        return block_stmt(self);

    else if (P_MATCH(self, TOKEN_TYPE_BREAK))
        return break_stmt(self);

    else if (P_MATCH(self, TOKEN_TYPE_CONTINUE))
        return continue_stmt(self);

    else if (P_MATCH(self, TOKEN_TYPE_RET))
        return ret_stmt(self);

    else
        return expr_stmt(self);
}


// Grammar
// expr_stmt -> expr ";" ;

void* expr_stmt(struct Parser* self) {
	#if defined(DEBUG)
		vi_log("expr_stmt()\n");
	#endif // DEBUG

    const struct Token *token = CUR_TOKEN(self);

    void *expression = expr(self);

    if (NULL == expression) return NULL;

    if (!P_CONSUME(self, TOKEN_TYPE_SEMICOLON)) {
        delete(expression);
        p_raise_error(
            self, token,
            "SyntaxError: Expected ';' after this expression."
        );
        return NULL;
    }

	#if defined(DEBUG)
		vi_log("ret expr_stmt()\n");
	#endif // DEBUG

    return new(ExprStmt, expression);
}

// Grammar
// if_stmt -> "if" "(" expr ")" stmt ( "else" stmt )? ;

void* if_stmt(struct Parser* self) {

#if defined(DEBUG)
    vi_log("if_stmt()\n");
#endif // DEBUG

    if (!P_MATCH(self, TOKEN_TYPE_IF)) {
        p_raise_error(
            self, CUR_TOKEN(self),
            "SyntaxError: Expected 'if' found '%.*s'.",
            CUR_TOKEN(self)->length, CUR_TOKEN(self)->start
        );
        return NULL;
    }

    const struct Token* if_token = CUR_TOKEN(self);
    P_ADVANCE(self);

    const struct Token* lparen_token = NULL;

    if (P_MATCH(self, TOKEN_TYPE_LPAREN)) {
        lparen_token = CUR_TOKEN(self);
        P_ADVANCE(self);
    }
    else {
        p_raise_error(
            self, if_token,
            "SyntaxError: Expected some expression in a pair of parenthesis after if."
        );
        return NULL;
    }

    void* expression = expr(self);

    if (NULL == expression) {
        p_raise_error(
            self, lparen_token,
            "SyntaxError: Expected some expression after '('."
        );
        return NULL;
    }

    if (!P_CONSUME(self, TOKEN_TYPE_RPAREN)) {
        delete(expression);

        p_raise_error(
            self, lparen_token,
            "SyntaxError: Expected corresponding ')' for '('."
        );

        return NULL;
    }

    void* if_block = declaration(self);

    if (NULL == if_block) {
        delete(expression);

        p_raise_error(
            self, if_token,
            "SyntaxError: Expected some statement for if."
        );

        return NULL;
    }

    void* else_block = NULL;

    if (P_MATCH(self, TOKEN_TYPE_ELSE)) {
        const struct Token* else_token = CUR_TOKEN(self);
        P_ADVANCE(self);

        else_block = stmt(self);

        if (NULL == else_block) {
            delete(if_block);
            delete(expression);

            p_raise_error(
                self, else_token,
                "SyntaxError: Expected some statement for else."
            );

            return NULL;
        }
    }

#if defined(DEBUG)
    vi_log("ret if_stmt()\n");
#endif // DEBUG

    return new(IfStmt, expression, if_block, else_block);
}

// Grammar
/*
when_stmt	->  "when" "(" expr ")" "{"
                    (term ("," term)* "->" stmt)+
                    ("else" -> stmt)?
                "}" ;
*/

void* when_case(struct Parser* self);

void* when_stmt(struct Parser* self) {

#if defined(DEBUG)
    vi_log("when_stmt()\n");
#endif // DEBUG

    if (!P_MATCH(self, TOKEN_TYPE_WHEN)) {
        p_raise_error(
            self, CUR_TOKEN(self),
            "SyntaxError: Expected 'when' found '%.*s'.",
            CUR_TOKEN(self)->length, CUR_TOKEN(self)->start
        );
        return NULL;
    }

    const struct Token* when_token = CUR_TOKEN(self);
    P_ADVANCE(self);

    const struct Token* lparen_token = NULL;
    const struct Token* rparen_token = NULL;
    const struct Token* lbrace_token = NULL;

    if (P_MATCH(self, TOKEN_TYPE_LPAREN)) {
        lparen_token = CUR_TOKEN(self);
        P_ADVANCE(self);
    }
    else {
        p_raise_error(
            self, when_token,
            "SyntaxError: Expected some expression in a pair of parenthesis after when."
        );

        return NULL;
    }

    void* condition_expr = expr(self);

    if (NULL == condition_expr) {
        p_raise_error(
            self, lparen_token,
            "SyntaxError: Expected some expression enclosed in parenthesis after when."
        );

        return NULL;
    }

    if (P_MATCH(self, TOKEN_TYPE_RPAREN)) {
        rparen_token = CUR_TOKEN(self);
        P_ADVANCE(self);
    }
    else {
        delete(condition_expr);

        p_raise_error(
            self, lparen_token,
            "SyntaxError: Expected corresponding ')' for '('."
        );

        return NULL;
    }

    if (P_MATCH(self, TOKEN_TYPE_LCURLY_BRACE)) {
        lbrace_token = CUR_TOKEN(self);
        P_ADVANCE(self);
    }
    else {
        delete(condition_expr);
        p_raise_error(
            self, rparen_token,
            "SyntaxError: Expected some case statements enclosed in curly braces after when's ')'."
        );

        return NULL;
    }

    void* parsed_when_case = when_case(self);

    if (NULL == parsed_when_case) {
        delete(condition_expr);

        p_raise_error(
            self, when_token,
            "SyntaxError: Expected some case statements after when's '{'."
        );

        return NULL;
    }

    uint32_t case_count = 0;
    void** cases = allocate(sizeof(void*), ++case_count);

    void* else_stmt = NULL;

    cases[case_count - 1] = parsed_when_case;

    while (!p_match_any(self, 3, TOKEN_TYPE_ELSE, TOKEN_TYPE_RCURLY_BRACE, TOKEN_TYPE_EOF)) {
        parsed_when_case = when_case(self);

        if (NULL == parsed_when_case)
            break;

        cases = reallocate(cases, sizeof(void*), ++case_count);
        cases[case_count - 1] = parsed_when_case;
    }

    cases = reallocate(cases, sizeof(void*), ++case_count);
    cases[case_count - 1] = NULL;


    if (P_MATCH(self, TOKEN_TYPE_ELSE)) {
        const struct Token* else_token = CUR_TOKEN(self);
        P_ADVANCE(self);

        const struct Token* arrow_token = NULL;

        if (P_MATCH(self, TOKEN_TYPE_DASH_ARROW)) {
            arrow_token = CUR_TOKEN(self);
            P_ADVANCE(self);
        }
        else {
            p_raise_error(
                self, else_token,
                "SyntaxError: Expected '->' after 'else'."
            );

            for (uint32_t i = 0; NULL != cases[i]; i++)
                delete(cases[i]);

            return NULL;
        }

        else_stmt = stmt(self);

        if (NULL == else_stmt) {
            p_raise_error(
                self, arrow_token,
                "SyntaxError: Expected some statement after '->'."
            );

            for (uint32_t i = 0; NULL != cases[i]; i++)
                delete(cases[i]);

            return NULL;
        }
    }

    if (!P_CONSUME(self, TOKEN_TYPE_RCURLY_BRACE)) {
        p_raise_error(
            self, lbrace_token,
            "SyntaxError: Expected corresponding '}' for '{'."
        );

        delete(condition_expr);

        for (uint32_t i = 0; NULL != cases[i]; i++)
            delete(cases[i]);

        if (NULL != else_stmt)
            delete(else_stmt);

        return NULL;
    }

#if defined(DEBUG)
    vi_log("ret when_stmt()\n");
#endif // DEBUG

    return new(WhenStmt, condition_expr, cases, else_stmt);
}


// Grammar
// when_case -> term ("," term)* "->" stmt

void* when_case(struct Parser* self) {

    #define FREE_EXPRS(exprs) \
		do { \
			for(uint32_t i = 0; exprs[i] != NULL; i++) \
				delete(exprs[i]);\
		} while(0) \

    void** case_exprs = NULL;
    uint32_t expr_count = 0;

    void* case_expr = term(self);

    if (NULL == case_expr)
        return NULL;

    case_exprs = reallocate(case_exprs, sizeof(void*), ++expr_count);
    case_exprs[expr_count - 1] = case_expr;

    const struct Token* comma_token = NULL;
    const struct Token* last_token = CUR_TOKEN(self);

    while (P_MATCH(self, TOKEN_TYPE_COMMA)) {
        comma_token = CUR_TOKEN(self);
        P_ADVANCE(self);

        last_token = CUR_TOKEN(self);
        case_expr = term(self);

        if (NULL == case_expr) {
            // got to deallocate exprs
            FREE_EXPRS(case_exprs);

            p_raise_error(
                self, comma_token,
                "SyntaxError: Expected some expression after ','."
            );

            return NULL;
        }

        case_exprs = reallocate(case_exprs, sizeof(void*), ++expr_count);
        case_exprs[expr_count - 1] = case_expr;
    }

    case_exprs = reallocate(case_exprs, sizeof(void*), ++expr_count);
    case_exprs[expr_count - 1] = NULL;


    const struct Token* arrow_token = NULL;

    if (P_MATCH(self, TOKEN_TYPE_DASH_ARROW)) {
        arrow_token = CUR_TOKEN(self);
        P_ADVANCE(self);
    }
    else {
        FREE_EXPRS(case_exprs);

        p_raise_error(
            self, last_token,
            "SyntaxError: Expected '->' after expression."
        );

        return NULL;
    }

    void* case_stmt = stmt(self);

    if (NULL == case_stmt) {
        FREE_EXPRS(case_exprs);

        p_raise_error(
            self, arrow_token,
            "SyntaxError: Expected some statement after '->'."
        );

        return NULL;
    }

    return new(WhenCase, case_exprs, case_stmt);

    #undef FREE_EXPRS
}





// Grammar
// loop_stmt -> 'loop' '(' (IDENTIFIER ':')? term ('->' | '=>') term (';' term)? ')' stmt	|
//				'loop' '(' expr ')' stmt ;

void* loop_stmt(struct Parser* self) {

#if defined(DEBUG)
    vi_log("loop_stmt()\n");
#endif // DEBUG

    if (!P_MATCH(self, TOKEN_TYPE_LOOP)) {
        p_raise_error(
            self, CUR_TOKEN(self),
            "SyntaxError: Expected 'loop' found '%.*s'.",
            CUR_TOKEN(self)->length, CUR_TOKEN(self)->start
        );
        return NULL;
    }

    const struct Token* loop_token = CUR_TOKEN(self);
    P_ADVANCE(self);

    const struct Token* lparen_token = NULL;

    if (P_MATCH(self, TOKEN_TYPE_LPAREN)) {
        lparen_token = CUR_TOKEN(self);
        P_ADVANCE(self);
    }
    else {
        p_raise_error(
            self, loop_token,
            "SyntaxError: Expected '(' after loop."
        );
        return NULL;
    }

    const struct Token* arrow_token = NULL;
    const struct Token* id_token = NULL;

    enum LoopStmtType loop_type;

    void* start_expr = expr(self);
    void* end_expr = NULL;
    void* step_expr = NULL;

    if (NULL == start_expr) {
        p_raise_error(
            self, lparen_token,
            "SyntaxError: Expected some expression after '('."
        );
        return NULL;
    }

    if (P_MATCH(self, TOKEN_TYPE_RPAREN)) {
        P_ADVANCE(self);

        loop_type = LOOP_STMT_TYPE_CONDITIONAL;
    }
    else {
        loop_type = LOOP_STMT_TYPE_RANGED;

        if (p_match_any(self, 2, TOKEN_TYPE_DASH_ARROW, TOKEN_TYPE_EQUAL_ARROW)) {
            arrow_token = CUR_TOKEN(self);
            P_ADVANCE(self);
        }
        else {
            delete(start_expr);

            p_raise_error(
                self, loop_token,
                "SyntaxError: Expected '->' or '=>' after expression for loop."
            );

            return NULL;
        }

        end_expr = term(self);

        if (NULL == end_expr) {
            delete(start_expr);
            return NULL;
        }

        if (P_MATCH(self, TOKEN_TYPE_SEMICOLON)) {
            const struct Token* semicolon = CUR_TOKEN(self);
            P_ADVANCE(self);

            step_expr = term(self);

            if (NULL == step_expr) {
                delete(start_expr);
                delete(end_expr);

                p_raise_error(
                    self, semicolon,
                    "SyntaxError: Expected some step value after ';'."
                );

                return NULL;
            }
        }

        if (P_MATCH(self, TOKEN_TYPE_RPAREN)) {
            P_ADVANCE(self);
        }
        else {
            delete(start_expr);
            delete(end_expr);

            if (NULL != step_expr)
                delete(step_expr);

            p_raise_error(
                self, lparen_token,
                "SyntaxError: Expected corresponding ')' for '('."
            );

            return NULL;
        }
    }


    const void* loop_symbol_table = symtabstack_push(self->symbol_table_stack);

    #define LOOP_STMT_FREE(stmts, count) \
		do { \
			for(uint32_t i = 0; i < count; i++) \
				delete(stmts[i]); \
				\
			free(stmts); \
			symtabstack_pop(self->symbol_table_stack);	\
		} while(0)


    uint32_t child_stmts_count = 0;
    void** child_stmts = NULL;

    self->parsing_loop = true;

    if (P_MATCH(self, TOKEN_TYPE_LCURLY_BRACE)) {
        const struct Token* lbrace_token = CUR_TOKEN(self);
        P_ADVANCE(self);

        if (loop_type == LOOP_STMT_TYPE_RANGED) {
            if (
                P_MATCH(self, TOKEN_TYPE_IDENTIFIER) &&
                P_PEEK_NEXT(self, TOKEN_TYPE_DASH_ARROW)
            ) {
                id_token = CUR_TOKEN(self);
                P_ADVANCE(self);    // consume the id token
                P_ADVANCE(self);    // consume the '->' token
            }

            char* loop_iterator_id = NULL;

            if (NULL != id_token) {
                loop_iterator_id = allocate(sizeof(char), id_token->length + 1);
                strncpy(loop_iterator_id, id_token->start, id_token->length);
            }
            else {
                const char* it = "it";

                loop_iterator_id = allocate(sizeof(char), strlen(it) + 1);
                strncpy(loop_iterator_id, it, strlen(it));
            }

            void* symbol = new(Symbol, loop_iterator_id, loop_symbol_table, DEF_TYPE_VAR, NULL);
            symbol_table_define(loop_symbol_table, loop_iterator_id, symbol);

        }
        else {
            if (P_MATCH(self, TOKEN_TYPE_IDENTIFIER) &&
                P_PEEK_NEXT(self, TOKEN_TYPE_DASH_ARROW)) {

                id_token = CUR_TOKEN(self);

                delete(start_expr);
                symtabstack_pop(self->symbol_table_stack);	// loop_symbol_table

                p_raise_error(
                    self, id_token,
                    "SyntaxError: Unexpected '%.*s'.", id_token->length, id_token->start
                );

                return NULL;
            }
        }



        while (!p_match_any(self, 2, TOKEN_TYPE_EOF, TOKEN_TYPE_RCURLY_BRACE)) {
            void* child_stmt = declaration(self);

            if (NULL == child_stmt) {
                LOOP_STMT_FREE(child_stmts, child_stmts_count);
                return NULL;
            }

            child_stmts = reallocate(child_stmts, sizeof(void*), ++child_stmts_count);
            child_stmts[child_stmts_count - 1] = child_stmt;
        }

        if (!P_CONSUME(self, TOKEN_TYPE_RCURLY_BRACE)) {
            p_raise_error(
                self, lbrace_token,
                "SyntaxError: Unmatched '{', expected corresponding '}'."
            );

            LOOP_STMT_FREE(child_stmts, child_stmts_count);

            return NULL;
        }

        child_stmts = reallocate(child_stmts, sizeof(void*), ++child_stmts_count);
        child_stmts[child_stmts_count - 1] = NULL;
    }
    else {

        if (loop_type == LOOP_STMT_TYPE_RANGED) {
            const char* loop_iterator = "it";

            char* loop_it_id = allocate(sizeof(char), strlen(loop_iterator) + 1);
            strncpy(loop_it_id, loop_iterator, strlen(loop_iterator));

            void* symbol = new(Symbol, loop_it_id, loop_symbol_table, DEF_TYPE_VAR, NULL);
            symbol_table_define(loop_symbol_table, loop_it_id, symbol);
        }

        void* child_stmt = declaration(self);

        if (NULL == child_stmt) {
            symtabstack_pop(self->symbol_table_stack);	// loop_symbol_table
            return NULL;
        }

        child_stmts = allocate(sizeof(void*), 2);
        child_stmts[0] = child_stmt;
        child_stmts[1] = NULL;
    }

    symtabstack_pop(self->symbol_table_stack);	// loop_symbol_table

    self->parsing_loop = false;


#if defined(DEBUG)
    vi_log("ret loop_stmt()\n");
#endif // DEBUG

    return new(
        LoopStmt, loop_symbol_table, loop_type,
        id_token, start_expr, arrow_token, end_expr, step_expr,
        child_stmts
    );
}




// Grammar
// repeat_stmt -> "repeat" "(" (IDENTIFIER ':')? term ")" stmt ;

void* repeat_stmt(struct Parser* self) {

#if defined(DEBUG)
    vi_log("repeat_stmt()\n");
#endif // DEBUG

    if (!P_MATCH(self, TOKEN_TYPE_REPEAT)) {
        p_raise_error(
            self, CUR_TOKEN(self),
            "SyntaxError: Expected 'repeat' found '%.*s'.",
            CUR_TOKEN(self)->length, CUR_TOKEN(self)->start
        );
        return NULL;
    }

    const struct Token* repeat_token = CUR_TOKEN(self);
    P_ADVANCE(self);

    const struct Token* lparen_token = NULL;

    if (P_MATCH(self, TOKEN_TYPE_LPAREN)) {
        lparen_token = CUR_TOKEN(self);
        P_ADVANCE(self);
    }
    else {
        p_raise_error(
            self, repeat_token,
            "SyntaxError: Expected '(' after repeat."
        );
        return NULL;
    }

    const struct Token* id_token = NULL;

    void* expression = term(self);

    if (NULL == expression) {
        p_raise_error(
            self, lparen_token,
            "SyntaxError: Expected some expression after '('."
        );
        return NULL;
    }

    const struct Token* cur = CUR_TOKEN(self);

    if ((!P_MATCH(self, TOKEN_TYPE_EOF)) &&
        p_match_any(self, 8, TOKEN_TYPE_LOGICAL_AND, TOKEN_TYPE_LOGICAL_OR, TOKEN_TYPE_EQUAL_EQUAL, TOKEN_TYPE_NOT_EQUAL,
            TOKEN_TYPE_LESS_THAN, TOKEN_TYPE_LESS_THAN_EQUALS, TOKEN_TYPE_GREATER_THAN, TOKEN_TYPE_GREATER_THAN_EQUALS)
        ) {
        delete(expression);
        p_raise_error(
            self, cur,
            "Error: Unexpected '%.*s' in repeat's expression.", cur->length, cur->start
        );
        return NULL;
    }

    if (!P_MATCH(self, TOKEN_TYPE_RPAREN)) {
        delete(expression);

        p_raise_error(
            self, lparen_token,
            "Expected corresponding ')' for '('."
        );

        return NULL;
    }
    else {
        P_ADVANCE(self);
    }

    const void* repeat_symbol_table = symtabstack_push(self->symbol_table_stack);

	#define REPEAT_STMT_FREE(stmts, count) \
		do { \
			for(uint32_t i = 0; i < count; i++) \
				delete(stmts[i]); \
				\
			free(stmts); \
			symtabstack_pop(self->symbol_table_stack);	\
		} while(0)


    uint32_t child_stmts_count = 0;
    void** child_stmts = NULL;

    self->parsing_loop = true;

    if (P_MATCH(self, TOKEN_TYPE_LCURLY_BRACE)) {
        const struct Token* lbrace_token = CUR_TOKEN(self);
        P_ADVANCE(self);

        if (P_MATCH(self, TOKEN_TYPE_IDENTIFIER) &&
            P_PEEK_NEXT(self, TOKEN_TYPE_DASH_ARROW)) {

            id_token = CUR_TOKEN(self);
            P_ADVANCE(self);

            P_ADVANCE(self);
        }

        char* repeat_iterator_id = NULL;

        if (NULL != id_token) {
            repeat_iterator_id = allocate(sizeof(char), id_token->length + 1);
            strncpy(repeat_iterator_id, id_token->start, id_token->length);
        }
        else {
            const char* it = "it";

            repeat_iterator_id = allocate(sizeof(char), strlen(it) + 1);
            strncpy(repeat_iterator_id, it, strlen(it));
        }

        void* symbol = new(Symbol, repeat_iterator_id, repeat_symbol_table, DEF_TYPE_VAR, NULL);
        symbol_table_define(repeat_symbol_table, repeat_iterator_id, symbol);



        while (!p_match_any(self, 2, TOKEN_TYPE_EOF, TOKEN_TYPE_RCURLY_BRACE)) {
            void* child_stmt = declaration(self);

            if (NULL == child_stmt) {
                REPEAT_STMT_FREE(child_stmts, child_stmts_count);

                if (child_stmts_count == 0) {
                    p_raise_error(
                        self, lbrace_token,
                        "SyntaxError: Expected some statements after '{'."
                    );
                }

                return NULL;
            }

            child_stmts = reallocate(child_stmts, sizeof(void*), ++child_stmts_count);
            child_stmts[child_stmts_count - 1] = child_stmt;
        }

        if (!P_CONSUME(self, TOKEN_TYPE_RCURLY_BRACE)) {
            p_raise_error(
                self, lbrace_token,
                "SyntaxError: Expected corresponding '}' for '{'."
            );

            REPEAT_STMT_FREE(child_stmts, child_stmts_count);

            return NULL;
        }

        child_stmts = reallocate(child_stmts, sizeof(void*), ++child_stmts_count);
        child_stmts[child_stmts_count - 1] = NULL;
    }
    else {

        const char* it = "it";

        char* repeat_iterator_id = allocate(sizeof(char), strlen(it) + 1);
        strncpy(repeat_iterator_id, it, strlen(it));

        void* symbol = new(Symbol, repeat_iterator_id, repeat_symbol_table, DEF_TYPE_VAR, NULL);
        symbol_table_define(repeat_symbol_table, repeat_iterator_id, symbol);


        void* child_stmt = declaration(self);

        if (NULL == child_stmt) {
            symtabstack_pop(self->symbol_table_stack);	// repeat_symbol_table

            p_raise_error(
                self, repeat_token,
                "SyntaxError: Expected some child statement for repeat."
            );

            return NULL;
        }

        child_stmts = allocate(sizeof(void*), 2);
        child_stmts[0] = child_stmt;
        child_stmts[1] = NULL;
    }

    symtabstack_pop(self->symbol_table_stack);	// repeat_symbol_table

    self->parsing_loop = false;

#if defined(DEBUG)
    vi_log("ret repeat_stmt()\n");
#endif // DEBUG

    return new(RepeatStmt, repeat_symbol_table, id_token, expression, child_stmts);

#undef REPEAT_STMT_FREE
}


// Grammar
// block_stmt -> "{" declaration* "}" ;

void* block_stmt(struct Parser* self) {

#if defined(DEBUG)
    vi_log("block_stmt()\n");
#endif // DEBUG

    if (!P_MATCH(self, TOKEN_TYPE_LCURLY_BRACE)) {
        p_raise_error(
            self, CUR_TOKEN(self),
            "SyntaxError: Expected '{' found '%.*s'.",
            CUR_TOKEN(self)->length, CUR_TOKEN(self)->start
        );
        return NULL;
    }

#define BLOCK_STMT_FREE \
		do { \
			for(uint32_t i = 0; i < child_stmts_count; i++) \
				delete(child_stmts[i]); \
				\
			free(child_stmts); \
			symtabstack_pop(self->symbol_table_stack); \
		} while(0)


    const struct Token* lcurlybrace = CUR_TOKEN(self);
    P_ADVANCE(self);


    const void* block_stmt_symbol_table = symtabstack_push(self->symbol_table_stack);


    uint32_t child_stmts_count = 0;
    void** child_stmts = NULL;

    while (!p_match_any(self, 2, TOKEN_TYPE_RCURLY_BRACE, TOKEN_TYPE_EOF)) {
        void* child_stmt = declaration(self);

        if (NULL == child_stmt) {
            BLOCK_STMT_FREE;
            return NULL;
        }

        child_stmts = reallocate(child_stmts, sizeof(void*), ++child_stmts_count);
        child_stmts[child_stmts_count - 1] = child_stmt;
    }

    if (!P_CONSUME(self, TOKEN_TYPE_RCURLY_BRACE)) {
        BLOCK_STMT_FREE;

        p_raise_error(
            self, lcurlybrace,
            "SyntaxError: Expected corresponding '}' for '{'."
        );
        return NULL;
    }

    child_stmts = reallocate(child_stmts, sizeof(void*), ++child_stmts_count);
    child_stmts[child_stmts_count - 1] = NULL;


    symtabstack_pop(self->symbol_table_stack);	// block_stmt_symbol_table


#if defined(DEBUG)
    vi_log("ret block_stmt()\n");
#endif // DEBUG

    return new(BlockStmt, block_stmt_symbol_table, child_stmts);

#undef BLOCK_STMT_FREE
}






// Grammar
// break_stmt -> 'break' ';' ; 

void* break_stmt(struct Parser* self) {

#if defined(DEBUG)
    vi_log("break_stmt()\n");
#endif // DEBUG


    if (!P_MATCH(self, TOKEN_TYPE_BREAK)) {
        p_raise_error(
            self, CUR_TOKEN(self),
            "SyntaxError: Expected 'break' found '%.*s'.",
            CUR_TOKEN(self)->length, CUR_TOKEN(self)->start
        );
        return NULL;
    }

    const struct Token* break_token = CUR_TOKEN(self);
    P_ADVANCE(self);

    if (!self->parsing_loop) {
        p_raise_error(
            self, break_token,
            "Error: Break can be used only within a loop."
        );
        return NULL;
    }

    const struct Token* semicolon_token = NULL;

    if (!P_MATCH(self, TOKEN_TYPE_SEMICOLON)) {
        p_raise_error(
            self, break_token,
            "SyntaxError: Expected ';' afterwards."
        );
        return NULL;
    }
    else {
        semicolon_token = CUR_TOKEN(self);
        P_ADVANCE(self);
    }

#if defined(DEBUG)
    vi_log("ret break_stmt()\n");
#endif // DEBUG

    return new(BreakStmt, break_token, semicolon_token);
}


// Grammar
// continue_stmt -> 'continue' ';' ;

void* continue_stmt(struct Parser* self) {

#if defined(DEBUG)
    vi_log("continue_stmt()\n");
#endif // DEBUG

    if (!P_MATCH(self, TOKEN_TYPE_CONTINUE)) {
        p_raise_error(
            self, CUR_TOKEN(self),
            "SyntaxError: Expected 'continue' found '%.*s'.",
            CUR_TOKEN(self)->length, CUR_TOKEN(self)->start
        );
        return NULL;
    }

    const struct Token* continue_token = CUR_TOKEN(self);
    P_ADVANCE(self);

    if (!self->parsing_loop) {
        p_raise_error(
            self, continue_token,
            "Error: Continue can be used only within a loop."
        );
        return NULL;
    }

    const struct Token* semicolon_token = NULL;

    if (!P_MATCH(self, TOKEN_TYPE_SEMICOLON)) {
        p_raise_error(
            self, continue_token,
            "SyntaxError: Expected ';' afterwards."
        );
        return NULL;
    }
    else {
        semicolon_token = CUR_TOKEN(self);
        P_ADVANCE(self);
    }

#if defined(DEBUG)
    vi_log("ret continue_stmt()\n");
#endif // DEBUG

    return new(ContinueStmt, continue_token, semicolon_token);
}



// Grammar
// ret_stmt -> "ret" expr? ";" ;

void* ret_stmt(struct Parser* self) {

#if defined(DEBUG)
    vi_log("ret_stmt()\n");
#endif // DEBUG

    if (!P_MATCH(self, TOKEN_TYPE_RET)) {
        p_raise_error(
            self, CUR_TOKEN(self),
            "SyntaxError: Expected 'ret' found '%.*s'.",
            CUR_TOKEN(self)->length, CUR_TOKEN(self)->start
        );
        return NULL;
    }

    const struct Token* ret_token = CUR_TOKEN(self);
    P_ADVANCE(self);

    if (!self->parsing_fun) {
        p_raise_error(
            self, ret_token,
            "Error: Return can be used only within a funtion or method."
        );
        return NULL;
    }

    void* ret_expr = NULL;

    if (!P_MATCH(self, TOKEN_TYPE_SEMICOLON)) {
        ret_expr = expr(self);

        if (NULL == ret_expr) {
            return NULL;
        }
    }

    if (!P_CONSUME(self, TOKEN_TYPE_SEMICOLON)) {
        p_raise_error(
            self, ret_token,
            "SyntaxError: Expected ';' to end ret statement."
        );

        if (NULL != ret_expr)
            delete(ret_expr);

        return NULL;
    }

#if defined(DEBUG)
    vi_log("ret ret_stmt()\n");
#endif // DEBUG

    return new(RetStmt, ret_expr);
}









/*
    expr		->  assignment |
                    logical_or ;

    assignment	>	molecule ('=' | '+=' | '-=' | '*=' | '/=' | '%=') expr  ;
*/

// v1.1
void* expr(struct Parser* self) {

#if defined(DEBUG)
    vi_log("expr()\n");
#endif // DEBUG

    void* left_expr = logical_or(self);

    if (NULL == left_expr)
        return NULL;

    if (
        p_match_any(
            self, 6,
            TOKEN_TYPE_EQUAL, TOKEN_TYPE_PLUS_EQUAL, TOKEN_TYPE_MINUS_EQUAL,
            TOKEN_TYPE_STAR_EQUAL, TOKEN_TYPE_SLASH_EQUAL, TOKEN_TYPE_PERCENT_EQUAL
        )
    ) {
        const struct Token* equal_token = CUR_TOKEN(self);
        P_ADVANCE(self);

        if (
            !IS_IDENTIFIER_EXPR(left_expr) &&
            !IS_LIST_SUBSCRIPT_EXPR(left_expr) &&
            !IS_GET_EXPR(left_expr)
        ) {

            p_raise_error_expr(
                self, left_expr,
                "Error: Invalid assignment target."
            );
            delete(left_expr);
            return NULL;
        }

        void* right_expr = expr(self);

        if (IS_IDENTIFIER_EXPR(left_expr)) {
            struct IdentifierExpr* id_expr = left_expr;
            const enum DefinitionType def_type = SYMBOL_GET_DEF_TYPE(id_expr->symbol);

            if (DEF_TYPE_VAR != def_type) {
                if (DEF_TYPE_VAL == def_type) {
                    p_raise_error(
                        self, id_expr->id_token,
                        "Error: Invalid Assignment to a constant val '%.*s'.",
                        id_expr->id_token->length, id_expr->id_token->start
                    );

                    delete(left_expr);
                    return NULL;
                }

                p_raise_error(
                    self, id_expr->id_token,
                    "Error: Invalid assignment target."
                );

                delete(left_expr);
                return NULL;
            }

            void* id_assign_expr = new(
                IdentifierAssignExpr,
                id_expr->id_token, id_expr->symbol,
                equal_token, right_expr
                );
            free(id_expr);

#if defined(DEBUG)
            vi_log("ret expr()\n");
#endif // DEBUG

            return id_assign_expr;
        }

        else if (IS_LIST_SUBSCRIPT_EXPR(left_expr)) {
            struct ListSubscriptExpr* ls_expr = left_expr;

            if (IS_CALL_EXPR(ls_expr->left_expr)) {
                p_raise_error_expr(
                    self, ls_expr,
                    "Error: Invalid assignment target."
                );
                delete(left_expr);
                return NULL;
            }

            void* ls_assign_expr = new(
                ListSubscriptAssignExpr,
                ls_expr->left_expr, ls_expr->index_expr,
                equal_token, right_expr
                );

            free(ls_expr);

#if defined(DEBUG)
            vi_log("ret expr()\n");
#endif // DEBUG

            return ls_assign_expr;
        }

        else if (IS_GET_EXPR(left_expr)) {
            struct GetExpr* get_expr = left_expr;

            void* set_expr = new(
                SetExpr, get_expr->left_expr,
                get_expr->property_token, equal_token,
                right_expr
                );

            free(left_expr);


#if defined(DEBUG)
            vi_log("ret expr()\n");
#endif // DEBUG

            return set_expr;
        }
    }

#if defined(DEBUG)
    vi_log("ret expr()\n");
#endif // DEBUG

    return left_expr;
}



void* parse_binary(struct Parser* self, void* (*parse_fun)(struct Parser* self), uint32_t count, ...) {
    void* left = parse_fun(self);

    if (NULL == left)
        return NULL;

    va_list ap;
    va_start(ap, count);

    for (uint32_t i = 0; i < count; i++) {
        const struct TokenType expected = va_arg(ap, const struct TokenType);

        while (P_MATCH(self, expected)) {
            const struct Token* op_token = CUR_TOKEN(self);
            P_ADVANCE(self);

            void* right = parse_fun(self);

            if (NULL == right) {
                va_end(ap);
                delete(left);
                return NULL;
            }

            left = new(BinaryExpr, op_token, left, right);
        }
    }

    va_end(ap);

    return left;
}







// logical_or -> logical_and ( "||" logical_and )*

void* logical_or(struct Parser* self) {

#if defined(DEBUG)
    vi_log("logical_or()\n");
#endif // DEBUG

    void* left = logical_and(self);

    if (NULL == left)
        return NULL;

    while (P_MATCH(self, TOKEN_TYPE_LOGICAL_OR)) {
        const struct Token* op_token = CUR_TOKEN(self);
        P_ADVANCE(self);

        void* right = logical_and(self);

        if (NULL == right) {
            delete(left);
            return NULL;
        }

        left = new(LogicalExpr, op_token, left, right);
    }

#if defined(DEBUG)
    vi_log("ret logical_or()\n");
#endif // DEBUG

    return left;
}



// logical_and -> equality ("&&" equality )*

void* logical_and(struct Parser* self) {
    #if defined(DEBUG)
        vi_log("logical_and()\n");
    #endif // DEBUG

    void* left = equality(self);

    if (NULL == left)
        return NULL;

    while (P_MATCH(self, TOKEN_TYPE_LOGICAL_AND)) {
        const struct Token* op_token = CUR_TOKEN(self);
        P_ADVANCE(self);

        void* right = equality(self);

        if (NULL == right) {
            delete(left);
            return NULL;
        }

        left = new(LogicalExpr, op_token, left, right);
    }

#if defined(DEBUG)
    vi_log("ret logical_and()\n");
#endif // DEBUG

    return left;
}



// equality -> comparison ( ("==" | "!=") comparison )*

void* equality(struct Parser* self) {
#if defined(DEBUG)
    vi_log("call & ret equality()\n");
#endif // DEBUG

    return parse_binary(
        self, comparison, 2,
        TOKEN_TYPE_EQUAL_EQUAL, TOKEN_TYPE_NOT_EQUAL
    );
}



// comparison -> term  ( ( "<" | "<=" | ">" | ">=" ) term )?

void* comparison(struct Parser* self) {

#if defined(DEBUG)
    vi_log("comparison()\n");
#endif // DEBUG

    void* left = term(self);

    if (NULL == left)
        return NULL;

    if (
        p_match_any(
            self, 4,
            TOKEN_TYPE_LESS_THAN, TOKEN_TYPE_LESS_THAN_EQUALS,
            TOKEN_TYPE_GREATER_THAN, TOKEN_TYPE_GREATER_THAN_EQUALS
        )
        ) {
        const struct Token* op_token = CUR_TOKEN(self);
        P_ADVANCE(self);

        void* right = term(self);
        left = new(BinaryExpr, op_token, left, right);
    }

#if defined(DEBUG)
    vi_log("ret comparison()\n");
#endif // DEBUG

    return left;
}



// term -> factor (('+' | '-') factor)*

void* term(struct Parser* self) {
#if defined(DEBUG)
    vi_log("call & ret term()\n");
#endif // DEBUG

    return parse_binary(
        self, factor, 2,
        TOKEN_TYPE_PLUS, TOKEN_TYPE_MINUS
    );
}



// factor -> unary (('*' | '/' | '%') unary)*

void* factor(struct Parser* self) {

#if defined(DEBUG)
    vi_log("call & ret factor()\n");
#endif // DEBUG

    return parse_binary(
        self, unary, 3,
        TOKEN_TYPE_STAR, TOKEN_TYPE_FORWARD_SLASH, TOKEN_TYPE_PERCENT
    );
}


/*
unary	->	( "++" | "--" ) IDENTIFIER	|
            ('!' | '-' | '+')? unary	|
            molecule			;
*/
void* unary(struct Parser* self) {
#if defined(DEBUG)
    vi_log("unary()\n");
#endif // DEBUG

    if (p_match_any(self, 3, TOKEN_TYPE_NOT, TOKEN_TYPE_MINUS, TOKEN_TYPE_PLUS)) {
        const struct Token* op_token = CUR_TOKEN(self);
        P_ADVANCE(self);

        void* right = unary(self);

        if (NULL == right) {
            p_raise_error(
                self, op_token,
                "SyntaxError: Expected int, float or identifier after '%c'.", op_token->start[0]
            );

            return NULL;
        }

        void* expr = new(UnaryExpr, op_token, right);

#if defined(DEBUG)
        vi_log("ret unary()\n");
#endif // DEBUG

        return expr;
    }

    void* expr = molecule(self);

#if defined(DEBUG)
    vi_log("ret unary()\n");
#endif // DEBUG

    return expr;
}



/*
molecule	->  atom							|
                atom ( '(' arguments? ')' )*	|
                atom ( '.' IDENTIFIER )*		|
                atom ( '[' expr ']' )*			;

arguments	->	expr (',' expr)* ;
*/

void* molecule(struct Parser* self) {

#if defined(DEBUG)
    vi_log("molecule()\n");
#endif // DEBUG

    // const struct Token* start_token = CUR_TOKEN(self);

    void* left_expr = atom(self);

    if (NULL == left_expr) {
        return NULL;
    }

    bool first_time = true;
    const void* cur_instance_ptr_symbol = NULL;
    const void* super_class_symbol = NULL;

    while (1) {
        if (P_MATCH(self, TOKEN_TYPE_LPAREN)) {
			#if defined(DEBUG)
				vi_log("molecule(): matched an '('\n");
			#endif // DEBUG

            bool is_super_call = false;

            if (first_time) {
                first_time = false;

                if (!IS_IDENTIFIER_EXPR(left_expr)) {
                    p_raise_error_expr(
                        self, left_expr,
                        "Error: Target element is not callable."
                    );
                    delete(left_expr);
                    return NULL;
                }

                /*
                Not checking cuz variable's def type doesn't specify the value that it stores at runtime,
                it might store a FunValue
                */

                const struct IdentifierExpr* idexpr = left_expr;
                size_t super_len = strlen(super_name);

                if (
                    (super_len == idexpr->id_token->length) &&
                    (strncmp(idexpr->id_token->start, super_name, super_len) == 0)
				) {

                    if (!self->parsing_class) {
                        p_raise_error(
                            self, idexpr->id_token,
                            "Error: Invalid use of super."
                        );

                        delete(left_expr);

                        return NULL;
                    }

                    is_super_call = true;
                    cur_instance_ptr_symbol = symtabstack_lookup(self->symbol_table_stack, cur_instance_ptr_name);
                    super_class_symbol = symtabstack_lookup(self->symbol_table_stack, super_name);

                    if (NULL == cur_instance_ptr_symbol) {
                        p_raise_error(
                            self, idexpr->id_token,
                            "Error: Illegitimate use of super."
                        );

                        delete(left_expr);

                        return NULL;
                    }
                }
            }
            else {
                is_super_call = false;
                cur_instance_ptr_symbol = NULL;
                super_class_symbol = NULL;
            }

            const struct Token* lparen = CUR_TOKEN(self);
            P_ADVANCE(self);

#define CALL_EXPR_FREE(args, count) \
				do { \
					if(NULL != args) { \
						for(int i = 0; i < count; i++) { \
							if(NULL != args[i])		\
								delete(args[i]); 	\
						} \
					} \
					free(args); \
				} while(0)

            uint32_t arg_count = 0;
            void** arguments = NULL;

            if (!p_match_any(self, 2, TOKEN_TYPE_RPAREN, TOKEN_TYPE_EOF)) {
                void* arg = expr(self);

                if (NULL == arg) {
                    CALL_EXPR_FREE(arguments, arg_count);
                    return NULL;
                }

                arguments = reallocate(arguments, sizeof(void*), ++arg_count);
                arguments[arg_count - 1] = arg;

                while (P_MATCH(self, TOKEN_TYPE_COMMA)) {
                    const struct Token* comma_token = CUR_TOKEN(self);
                    P_ADVANCE(self);

                    arg = expr(self);

                    if (NULL == arg) {
                        CALL_EXPR_FREE(arguments, arg_count);

                        p_raise_error(
                            self, comma_token,
                            "SyntaxError: Expected expression after ','."
                        );

                        return NULL;
                    }

                    arguments = reallocate(arguments, sizeof(void*), ++arg_count);
                    arguments[arg_count - 1] = arg;
                }
            }

            const struct Token* rparen = NULL;

            if (P_MATCH(self, TOKEN_TYPE_RPAREN)) {
                rparen = CUR_TOKEN(self);
                P_ADVANCE(self);
            }
            else {
                CALL_EXPR_FREE(arguments, arg_count);

                p_raise_error(
                    self, lparen,
                    "SyntaxError: Unmatched '(', expected corresponding ')'."
                );
                return NULL;
            }

            if (is_super_call) {
                const struct IdentifierExpr* idexpr = left_expr;
                left_expr = new(SuperConstructorCallExpr, cur_instance_ptr_symbol, super_class_symbol, idexpr->id_token, lparen, arg_count, arguments, rparen);
            }
            else {
                left_expr = new(CallExpr, left_expr, lparen, arg_count, arguments, rparen);
            }

			#if defined(DEBUG)
				vi_log("molecule(): left is set to a CallExpr\n");
			#endif // DEBUG
        }

        else if (P_MATCH(self, TOKEN_TYPE_DOT)) {
			#if defined(DEBUG)
				vi_log("molecule(): matched a '.'\n");
			#endif // DEBUG

            bool is_super_get = false;

            if (first_time) {
                first_time = false;

                if (!IS_IDENTIFIER_EXPR(left_expr)) {
                    p_raise_error_expr(
                        self, left_expr,
                        "Error: Target element is not an object."
                    );
                    delete(left_expr);
                    return NULL;
                }

                struct IdentifierExpr* id_expr = left_expr;
                enum DefinitionType def_type = SYMBOL_GET_DEF_TYPE(id_expr->symbol);

                if (
                    (DEF_TYPE_VAR != def_type) && (DEF_TYPE_VAL != def_type) &&
                    (DEF_TYPE_CUR_INSTANCE_POINTER != def_type) && (DEF_TYPE_SUPER_PTR != def_type)
                ) {
                    p_raise_error_expr(
                        self, left_expr,
                        "Error: Target element is not an instance."
                    );
                    delete(left_expr);
                    return NULL;
                }

                // check whether id_token is 'super'
                size_t super_len = strlen(super_name);

                if (
                    (super_len == id_expr->id_token->length) &&
                    (strncmp(id_expr->id_token->start, super_name, super_len) == 0)
                ) {

                    if (!self->parsing_class) {
                        p_raise_error(
                            self, id_expr->id_token,
                            "Error: Invalid use of super."
                        );
                        delete(left_expr);
                        return NULL;
                    }

                    is_super_get = true;
                    cur_instance_ptr_symbol = symtabstack_lookup(self->symbol_table_stack, cur_instance_ptr_name);
                    super_class_symbol = symtabstack_lookup(self->symbol_table_stack, super_name);

					#if defined(DEBUG)
						if (NULL == cur_instance_ptr_symbol)
							vi_log("molecule(): cur_instance_ptr_symbol was NULL\n");
						if (NULL == super_class_symbol)
							vi_log("molecule(): super_class_symbol was NULL\n");
					#endif // DEBUG
                }
            }

            else {
                is_super_get = false;
                cur_instance_ptr_symbol = NULL;
                super_class_symbol = NULL;
            }

            // consume the dot_token matched in the enclosing if
            P_ADVANCE(self);

            const struct Token* property_token = NULL;

            if (P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
                property_token = CUR_TOKEN(self);
                P_ADVANCE(self);
            }
            else {
                p_raise_error(
                    self, PREV_TOKEN(self),
                    "SyntaxError: Expected some field after '.'."
                );
                delete(left_expr);
                return NULL;
            }

            // check if property accessed is 'constructor'
            size_t constructor_name_len = strlen(constructor_name);

            if (
                (property_token->length == constructor_name_len) &&
                (strncmp(constructor_name, property_token->start, constructor_name_len) == 0)
			) {
                p_raise_error(
                    self, property_token,
                    "Error: Invalid explicit access of constructor."
                );

                delete(left_expr);
                return NULL;
            }

            if (is_super_get) {
                is_super_get = false;
                const struct IdentifierExpr* id_expr = left_expr;
                left_expr = new(SuperGetExpr, cur_instance_ptr_symbol, super_class_symbol, id_expr->id_token, property_token);
            }
            else {
                left_expr = new(GetExpr, left_expr, property_token);
            }

			#if defined(DEBUG)
				vi_log("molecule(): left is set to a GetExpr | SuperGetExpr\n");
			#endif // DEBUG
        }

        else if (P_MATCH(self, TOKEN_TYPE_LSQUARE_BRACE)) {
			#if defined(DEBUG)
				vi_log("molecule(): matched a '['\n");
			#endif // DEBUG

            if (first_time) {
                first_time = false;

                if (!IS_IDENTIFIER_EXPR(left_expr)) {
                    p_raise_error_expr(
                        self, left_expr,
                        "Error: Invalid Subscripted element."
                    );
                    delete(left_expr);
                    return NULL;
                }

                struct IdentifierExpr* id_expr = left_expr;
                enum DefinitionType def_type = SYMBOL_GET_DEF_TYPE(id_expr->symbol);

                if ((DEF_TYPE_VAR != def_type) && (DEF_TYPE_VAL != def_type)) {
                    p_raise_error_expr(
                        self, left_expr,
                        "Error: Subscripted element is not a variable."
                    );
                    delete(left_expr);
                    return NULL;
                }
            }

            const struct Token* lsbrace = CUR_TOKEN(self);
            P_ADVANCE(self);

            void* index_expr = expr(self);

            if (NULL == index_expr) {
                delete(left_expr);
                return NULL;
            }

            const struct Token* rsbrace = NULL;

            if (P_MATCH(self, TOKEN_TYPE_RSQUARE_BRACE)) {
                rsbrace = CUR_TOKEN(self);
                P_ADVANCE(self);
            }
            else {
                p_raise_error(
                    self, lsbrace,
                    "SyntaxError: Unmatched '[', expected corressponding ']'."
                );
                return NULL;
            }

            left_expr = new(ListSubscriptExpr, left_expr, lsbrace, index_expr, rsbrace);

			#if defined(DEBUG)
				vi_log("molecule(): left is set to a ListSubscriptExpr\n");
			#endif // DEBUG
        }

        else {
            break;
        }
    }

	#if defined(DEBUG)
		vi_log("ret molecule()\n");
	#endif // DEBUG

    return left_expr;
}






/*
atom -> INTEGER | DOUBLE		|
        "true"	| "false"		|
        STRING					|
        IDENTIFIER				|
        "(" expr ")"			|	// group expr
        "[" expr ("," expr)* "]";	// list expr
*/

void* atom(struct Parser* self) {
	#if defined(DEBUG)
		vi_log("atom()\n");
	#endif // DEBUG

    if (
        P_MATCH(self, TOKEN_TYPE_INTEGER) || P_MATCH(self, TOKEN_TYPE_DOUBLE) ||
        P_MATCH(self, TOKEN_TYPE_TRUE) || P_MATCH(self, TOKEN_TYPE_FALSE) ||
        P_MATCH(self, TOKEN_TYPE_STRING)
	) {
        P_ADVANCE(self);
		#if defined(DEBUG)
			vi_log("ret LiteralExpr  atom()\n");
		#endif // DEBUG
        return new(LiteralExpr, PREV_TOKEN(self));
    }

    else if (P_MATCH(self, TOKEN_TYPE_IDENTIFIER)) {
        const struct Token* id_token = CUR_TOKEN(self);
        P_ADVANCE(self);

        char* id = allocate(sizeof(char), id_token->length + 1);
        strncpy(id, id_token->start, id_token->length);

        const void* id_symbol = symtabstack_lookup(self->symbol_table_stack, id);

        if (NULL == id_symbol) {
            size_t super_len = strlen(super_name);

            if ((strlen(id) == super_len) && (strncmp(id, super_name, super_len) == 0)) {
                if (!self->parsing_class) {
                    p_raise_error(
                        self, id_token,
                        "Error: Invalid use of 'super'."
                    );
                }
                else if (!self->parsing_class_has_superclass) {
                    p_raise_error(
                        self, id_token,
                        "Error: Invalid use of 'super', class has no super class."
                    );
                }
            }
            else {
                p_raise_error(
                    self, id_token,
                    "Error: '%.*s' is undefined.", id_token->length, id_token->start
                );
            }

            return NULL;
        }

		#if defined(DEBUG)
			vi_log("ret (IdentifierExpr, '%s') atom()\n", id);
		#endif // DEBUG

        free(id);

        return new(IdentifierExpr, id_token, id_symbol);
    }

    else if (P_MATCH(self, TOKEN_TYPE_LPAREN)) {
        const struct Token* lparen = CUR_TOKEN(self);
        P_ADVANCE(self);

        void* child_expr = expr(self);

        if (NULL == child_expr)
            return NULL;

        const struct Token* rparen = NULL;

		if (!P_MATCH(self, TOKEN_TYPE_RPAREN)) {
			p_raise_error(
				self, lparen,
				"SyntaxError: Expected corresponding ')' for '('."
			);
			delete(child_expr);
			return NULL;
		}
		else {
			rparen = CUR_TOKEN(self);
			P_ADVANCE(self);
		}

		#if defined(DEBUG)
			vi_log("ret GroupExpr atom()\n");
		#endif // DEBUG

        return new(GroupExpr, lparen, child_expr, rparen);
    }

    else if (P_MATCH(self, TOKEN_TYPE_LSQUARE_BRACE)) {
        const struct Token* lsbrace = CUR_TOKEN(self);
        P_ADVANCE(self);

		#define LIST_FREE(expressions, expr_count) \
			do { \
				for(uint64_t i = 0; i < expr_count; i++) \
					delete(expressions[i]); \
				free(expressions); \
			} while(0);

        uint32_t expr_count = 0;
        void** expressions = NULL;

		if(!P_MATCH(self, TOKEN_TYPE_RSQUARE_BRACE)) {
			do {
				void *child_expr = expr(self);

				if (NULL == child_expr) {
					LIST_FREE(expressions, expr_count);
					return NULL;
				}

				expressions = reallocate(expressions, sizeof(void *), ++expr_count);
				expressions[expr_count - 1] = child_expr;

			} while(P_CONSUME(self, TOKEN_TYPE_COMMA));
		}

		/*
        void* child_expr = expr(self);

        if (NULL == child_expr)
            return NULL;

        expressions = allocate(sizeof(void*), ++expr_count);
        expressions[expr_count - 1] = child_expr;

        while (P_MATCH(self, TOKEN_TYPE_COMMA)) {
            const struct Token* comma_token = CUR_TOKEN(self);
            P_ADVANCE(self);

            void *child_expr = expr(self);

            if (NULL == child_expr)
                return NULL;

            expressions = reallocate(expressions, sizeof(void*), ++expr_count);
            expressions[expr_count - 1] = child_expr;
        }
		// */

        const struct Token* rsbrace = NULL;

        if (!P_MATCH(self, TOKEN_TYPE_RSQUARE_BRACE)) {
            p_raise_error(
                self, lsbrace,
                "SyntaxError: Unmatched '[', expected corresponding ']'."
            );
			LIST_FREE(expressions, expr_count);
            return NULL;
        }
        else {
            rsbrace = CUR_TOKEN(self);
            P_ADVANCE(self);
        }

		#if defined(DEBUG)
			vi_log("ret ListExpr atom()\n");
		#endif // DEBUG

        return new(ListExpr, lsbrace, expr_count, expressions, rsbrace);
    }

    return NULL;
}









void parse(void* _self, bool is_import, const void* lexer) {
	#if defined(DEBUG)
		vi_log("parse()\n");
	#endif // DEBUG

	/*
	struct timespec start_time, end_time, delta_time;
	clock_gettime(CLOCK_REALTIME, &start_time);
	*/

	struct Parser* self = _self;
	assert(IS_PARSER(self));
	assert(IS_LEXER(lexer));

	self->is_import = is_import;
	self->tokens = LEXER_GET_TOKENS(lexer);
	self->tokens_count = LEXER_TOKENS_COUNT(lexer);

	self->symbol_table_stack = new(SymbolTableStack);

	assert(predef_symbol_table);
	symtabstack_push_table(self->symbol_table_stack, predef_symbol_table);
	declare_native_funs();

	self->program_ast = program(self);

	/*
	clock_gettime(CLOCK_REALTIME, &end_time);
	calc_delta_time(&start_time, &end_time, &delta_time);

	char time_string[64];
	put_formatted_time(time_string, &delta_time, 64);

	puts("");
	printf("|====================>\n");
	printf("| Parser Summary\n");
	printf("|====================>\n");
	printf("| File: '%s'\n", SOURCE_INSTREAM(self->source));
	printf("| Total Parsing time: '%s'\n", time_string);
	printf("| Start time..: %lds %.9ldns\n", (int)start_time.tv_sec, start_time.tv_nsec);
	printf("| End time....: %lds %.9ldns\n", (int)end_time.tv_sec, end_time.tv_nsec);
	printf("| Delta time..: %lds %.9ldns\n", (int)delta_time.tv_sec, delta_time.tv_nsec);
	printf("|========================================>\n\n");
	*/

	#if defined(DEBUG)
		vi_log("ret parse()\n");
	#endif // DEBUG
}

