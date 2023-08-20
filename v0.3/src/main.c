#include <assert.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "log.h"
#include "object.h"

#include "headers_frontend.h"
#include "my_interpreter.h"

#include "predefined.h"


static int PROMPT_MODE = 0;

int is_prompt_mode() {
	return PROMPT_MODE;
}


/*
void command_prompt() {
	void *lexer;
	void *parser;
	void *syntax_analyser;
	void *interpreter;
	void *source;

	while(1) {
		source = new(Source);

		if(!source_read_stream(source, stdin)) {
			delete(source);
			continue;
		}

		lexer = new(Lexer, source);
		scan(lexer);

		if(LEXER_HAD_ERROR(lexer)) {
			delete(lexer);
			lexer = NULL;
			continue;
		}

		parser = new(Parser, source);
		parse(parser, false, lexer);

		if(PARSER_HAD_ERROR(parser)) {
			delete(parser);
			parser = NULL;
			continue;
		}

		interpreter = new(Interpreter, source);
		interpret(interpreter, parser);

		delete(source);
		source = NULL;

		delete(lexer);
		lexer = NULL;

		destroy_predef_table();
		delete(parser);
		parser = NULL;

		destroy_predef_frame();
		delete(interpreter);
		interpreter = NULL;
	}
}
// */

void interpret_file(const char *path) {

	void *source = new(Source, path);
	source_read_file(source, path);

	#if defined(DEBUG)
		open_log_stream("log/log.txt");
	#endif // DEBUG

	void *lexer = new(Lexer, source);
	scan(lexer);

	if(LEXER_HAD_ERROR(lexer)) {
		#if defined(DEBUG)
			vi_log("LEXER had error\n");
			close_log_stream();
		#endif // DEBUG

		delete(source);
		delete(lexer);
		return;
	}

	void *parser = new(Parser, source);
	init_predef_table();
	parse(parser, false, lexer);

	if(PARSER_HAD_ERROR(parser)) {
		#if defined(DEBUG)
			vi_log("Parser had error\n");
			close_log_stream();
		#endif // DEBUG

		delete(source);
		delete(lexer);
		delete(parser);
		return;
	}

	void *interpreter = new(Interpreter, source);
	init_predef_frame();
	interpret(interpreter, parser);

	delete(source);
	source = NULL;

	delete(lexer);
	lexer = NULL;

	destroy_predef_table();
	delete(parser);
	parser = NULL;

	destroy_predef_frame();
	delete(interpreter);
	interpreter = NULL;
	
	#if defined(DEBUG)
		close_log_stream();
	#endif // DEBUG
}

int main(int argc, char const *argv[]) {

	// if(argc == 1) {
	// 	PROMPT_MODE = 1;
	// 	command_prompt();
	// }
	// else if(argc == 2) {
	// 	interpret_file(argv[1]);
	// }
	// else {
	// 	printf("Usage: main [path]");
	// }

	if(argc == 1) {
		printf("\n Usage: <Interpreter> [FilePath]\n");
		printf("     [FilePath] - The path of the file to execute.\n\n");
	}
	else if(argc == 2) {
		interpret_file(argv[1]);
	}
	
	return 0;
}


 