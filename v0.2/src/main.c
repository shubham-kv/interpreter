#include <assert.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <time.h>

#include "log.h"
#include "object.h"

#include "token_type.h"
#include "my_lexer.h"
#include "my_parser.h"
#include "my_syntax_analyser.h"

#include "interpreter.h"

// #include "ast_printer.h"



static int PROMPT_MODE = 0;

int is_prompt_mode() {
	return PROMPT_MODE;
}



void command_prompt() {
	void *lexer;
	void *parser;
	void *syntax_analyser;
	void *interpreter;
	// void *ast_printer;

	const int BUFFER_SIZE = 256;

	char **lines = calloc(2, sizeof(char *));
	char *buffer = calloc(BUFFER_SIZE + 1, sizeof(char));
	lines[0] = buffer;
	lines[1] = NULL;


	while(1) {
		fflush(stdin);
		memset(buffer, 0, BUFFER_SIZE + 1);

		printf(">> ");
		fgets(buffer, BUFFER_SIZE, stdin);

		if(strlen(buffer) <= 1) continue;

		lexer = new(Lexer, (const char *const *)lines);
		scan(lexer);

		if(LEXER_HAD_ERROR(lexer)) {
			delete(lexer);
			lexer = NULL;
			continue;
		}


		// printf("TOKENS\n");
		// const void *const *tokens = LEXER_GET_TOKENS(lexer);

		// for(uint32_t i = 0; NULL != tokens[i]; i++)
		// 	print_token(tokens[i], stdout);

		// printf("\n");

		// syntax_analyser = new(SyntaxAnalyser, lexer);
		// perform_analysis(syntax_analyser);

		parser = new(Parser, lexer);
		parse(parser);

		if(PARSER_HAD_ERROR(parser)) {
			delete(parser);
			parser = NULL;
			continue;
		}

		// printf("\n");
		// printf("|=========================>\n");
		// printf("| Abstract Syntax tree\n");
		// printf("|=========================>\n\n");

		// ast_printer = new(AstPrinter, parser_get_stmts(parser), false, 4);
		// print_ast(ast_printer);

		// printf("\n");

		// printf("\n");
		// printf("|=========================>\n");
		// printf("| Output\n");
		// printf("|=========================>\n\n");

		const void *program_ast = PARSER_GET_PROGRAM_AST(parser);
		const void *symtabstack = PARSER_GET_SYMBOL_TABLE_STACK(parser);
	
		interpreter = new(Interpreter, lexer, program_ast, symtabstack);
		interpret(interpreter);

		delete(lexer);
		// delete(syntax_analyser);
		delete(parser);
		delete(interpreter);
		// delete(ast_printer);

		lexer = NULL;
		// syntax_analyser = NULL;
		parser = NULL;
		interpreter = NULL;
		// ast_printer = NULL;
	}
}


const char *const *read_file(const char *path) {
	FILE *fp = fopen(path, "rb");

	if(NULL == fp) {
		fprintf(stderr, "Cannot read file '%s'.\n", path);
		exit(EXIT_FAILURE);
	}

	#define ARR_STEP_SIZE 128
	#define BUFFER_SIZE 1024

	int i = 0;
	int arr_size = 0;

	// A pointer to array of (char *)s;
	char **lines = NULL;

	// The buffer where each line from the file is read into;
	char line_buffer[BUFFER_SIZE];

	while(fgets(line_buffer, BUFFER_SIZE, fp)) {
		if(i == arr_size) {
			arr_size += ARR_STEP_SIZE;

			char **new_lines = realloc(lines, (arr_size * sizeof(char *)));

			if(NULL == new_lines) {
				fprintf(stderr, "Not enough memory to load file '%s'.\n", path);
				exit(EXIT_FAILURE);
			}

			lines = new_lines;
		}

		// +1 for null byte;
		size_t cur_line_size = strlen(line_buffer) + 1;
		lines[i] = calloc(cur_line_size, sizeof(char *));

		strcpy(lines[i], line_buffer);

		i++;
	}

	fclose(fp);

	if(i == arr_size) {
		char **new_lines = realloc(lines, ((arr_size + 1) * sizeof(char *)));

		if(NULL == new_lines) {
			fprintf(stderr, "Not enough memory to load file '%s'.\n", path);
			exit(EXIT_FAILURE);
		}

		lines = new_lines;
	}

	lines[i] = NULL;

	return (const char *const *)lines;

	#undef ARR_STEP_SIZE
	#undef BUFFER_SIZE
}


void interpret_file(const char *path) {
	const char *const *source = read_file(path);

	#if defined(DEBUG)
		open_log_stream("C:\\Users\\prince\\CProgramming\\Projects\\Interpreter\\log\\log.txt");

		vi_log("\n\n");
		vi_log("|====================>\n");
		vi_log("| LEXER\n");
		vi_log("|====================>\n\n");

	#endif // DEBUG

	void *lexer = new(Lexer, source);
	scan(lexer);

	if(LEXER_HAD_ERROR(lexer)) {
		delete(lexer);
		return;
	}

	#if defined(DEBUG)
		vi_log("\n\n");
		vi_log("|====================>\n");
		vi_log("| PARSER\n");
		vi_log("|====================>\n\n");

	#endif // DEBUG

	void *parser = new(Parser, lexer);
	parse(parser);

	if(PARSER_HAD_ERROR(parser)) {
		delete(parser);
		return;
	}


	#if defined(DEBUG)
		vi_log("\n\n");
		vi_log("|====================>\n");
		vi_log("| INTERPRETER\n");
		vi_log("|====================>\n\n");

	#endif // DEBUG

	const void *program_ast = PARSER_GET_PROGRAM_AST(parser);
	const void *symtabstack = PARSER_GET_SYMBOL_TABLE_STACK(parser);

	void *interpreter = new(Interpreter, lexer, program_ast, symtabstack);
	interpret(interpreter);

	delete(lexer);
	lexer = NULL;

	// delete(syntax_analyser);
	// syntax_analyser = NULL;

	delete(parser);
	parser = NULL;

	delete(interpreter);
	interpreter = NULL;
	

	for(int i = 0; NULL != source[i]; i++)
		free((char *) source[i]);

	free((char **) source);
	source = NULL;

	#if defined(DEBUG)
		close_log_stream();
	#endif // DEBUG
}

int main(int argc, char const *argv[]) {

	if(argc == 1) {
		PROMPT_MODE = 1;
		command_prompt();
	}
	else if(argc == 2) {
		interpret_file(argv[1]);
	}
	else {
		printf("Usage: main [path]");
	}
	
	return 0;
}


 