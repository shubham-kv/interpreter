// #include "ast_printer.h"

// #include <stdio.h>
// #include <stdlib.h>
// #include <stdint.h>
// #include <stdarg.h>
// #include <assert.h>
// #include <string.h>

// #include "token_type.h"
// #include "token.h"
// #include "value.h"

// #include "ast_node_headers.h"

// #include "ast_object.h"
// #include "expr_evaluator.h"
// #include "stmt_executor.h"

// #include "object.h"
// #include "type.h"


// struct AstPrinter {
// 	const void *class;

// 	const void *expr_visitor;

// 	const void *stmt_visitor;

// 	// const void *expr;

// 	const void *const *stmts;

// 	bool space_is_tab;

// 	int tab_width;	// only useful when printing ' ' instead of '\t'

// 	int tabs;
// };

// static void *astprinter_constructor(void *self, va_list *ap);
// static void *astprinter_destructor(void *self);

// static const struct Class _class_ast_printer = {
// 	sizeof(struct AstPrinter),
// 	astprinter_constructor,
// 	astprinter_destructor
// };

// const void *AstPrinter = &_class_ast_printer;


// static void print_var_decl		(const void *visitor, const void *var_decl);

// static void print_expr_stmt		(const void *visitor, const void *expr_stmt);
// static void print_print_stmt	(const void *visitor, const void *print_stmt);

// static void print_if_stmt		(const void *visitor, const void *if_stmt);
// static void print_when_stmt		(const void *visitor, const void *when_stmt);

// static void print_loop_stmt		(const void *visitor, const void *loop_stmt);
// static void print_repeat_stmt	(const void *visitor, const void *repeat_stmt);

// static void print_block_stmt	(const void *visitor, const void *block_stmt);






// static const void *print_identifier_expr(const void *visitor, const void *iexpr);
// static const void *print_literal_expr(const void *visitor, const void *lexpr);

// static const void *print_unary_expr		(const void *visitor, const void *uexpr);
// static const void *print_binary_expr	(const void *visitor, const void *bexpr);
// static const void *print_group_expr		(const void *visitor, const void *gexpr);

// static const void *print_var_assign_expr(const void *visitor, const void *vaexpr);


// // ... = new(AstPrinter, stmts, true);
// // ... = new(AstPrinter, stmts, false, 4);

// static void *astprinter_constructor(void *_self, va_list *ap) {
// 	struct AstPrinter *self = _self;
// 	assert(self && (self->class == AstPrinter));

// 	self->expr_visitor = new(
// 		ExprEvaluator,
// 		print_identifier_expr,
// 		print_literal_expr,
// 		print_unary_expr,
// 		print_binary_expr,
// 		print_group_expr,
// 		print_var_assign_expr
// 	);

// 	self->stmt_visitor = new(
// 		StmtExecutor,
// 		print_var_decl,
// 		print_expr_stmt, 	print_print_stmt,
// 		print_if_stmt,		print_when_stmt,
// 		print_loop_stmt,	print_repeat_stmt,
// 		print_block_stmt
// 	);

// 	self->stmts = va_arg(*ap, const void *const *);

// 	self->space_is_tab = (bool)va_arg(*ap, const int);

// 	if(self->space_is_tab) {
// 		self->tab_width = 4;
// 	} else {
// 		self->tab_width = va_arg(*ap, const int);
// 	}

// 	self->tabs = 0;

// 	return (void *)self;
// }

// static void *astprinter_destructor(void *_self) {
// 	struct AstPrinter *self = _self;
// 	assert(self && (self->class == AstPrinter));

// 	delete((void *) self->expr_visitor);
// 	delete((void *) self->stmt_visitor);

// 	memset(self, 0, sizeof(struct AstPrinter));

// 	return (void *)self;
// }

// static struct AstPrinter *printer;



// static void print_stmt(const void *stmt_visitor, const void *stmt) {
// 	accept_executor(stmt, stmt_visitor);
// }

// static const void *print_expr(const void *expr_visitor, const void *expr) {
// 	return accept_evaluator(expr, expr_visitor);
// }




// // prints the spaces before tag
// #define PRINT_SPACES(self) \
// 	do { \
// 		for(int i=0; i < self->tabs; i++) { \
// 			if(self->space_is_tab) { \
// 				printf("\t"); \
// 			} else { \
// 				for(int j=0; j < self->tab_width; j++) printf(" "); \
// 			} \
// 		} \
// 	} while(0)



// static void print_expr_stmt(const void *visitor, const void *expr_stmt) {

// }



// static void print_print_stmt(const void *visitor, const void *print_stmt) {
// 	struct AstPrinter *self = printer;
// 	assert(self && print_stmt && (self->class == AstPrinter));

// 	PRINT_SPACES(self);

// 	printf("<PrintStmt>\n");
// 	self->tabs++;

// 	const void *child_expr = print_stmt_get_expr(print_stmt);
// 	print_expr(printer->expr_visitor, child_expr);

// 	if(self->tabs > 0)
// 		self->tabs--;

// 	PRINT_SPACES(self);
// 	printf("</PrintStmt>\n");
// }


// /*

// if(true) {
// 	print "foo";
// }
// else {
// 	print "bar";
// }

// <IfStmt>
// 	<Condition>
// 		<LiteralExpr>
// 			<Value
// 				type="BOOLEAN"
// 				value="true" />
// 		</LiteralExpr>
// 	</Condition>

// 	<IfBlock>
// 		<BlockStmt>
// 			<PrintStmt>
// 				<LiteralExpr>
// 					<Value
// 						type="STRING"
// 						value="foo" />
// 				</LiteralExpr>
// 			</PrintStmt>
// 		</BlockStmt>
// 	</IfBlock>

// 	<ElseBlock>
// 		<BlockStmt>
// 			<PrintStmt>
// 				<LiteralExpr>
// 					<Value
// 						type="STRING"
// 						value="bar" />
// 				</LiteralExpr>
// 			</PrintStmt>
// 		</BlockStmt>
// 	</ElseBlock>
// </IfStmt>

// */


// static void print_var_decl(const void *visitor, const void *var_decl) {

// }




// static void print_if_stmt(const void *visitor, const void *if_stmt) {
// 	struct AstPrinter *self = printer;
// 	assert(self && if_stmt && (self->class == AstPrinter));


// 	PRINT_SPACES(self);

// 	printf("<IfStmt>\n");
// 	self->tabs++;

// 	const void *condition_expr = if_stmt_get_expr(if_stmt);

// 	PRINT_SPACES(self);
// 	printf("<Condition>\n");
// 	self->tabs++;
// 	print_expr(printer->expr_visitor, condition_expr);

// 	if(self->tabs > 0) self->tabs--;

// 	PRINT_SPACES(self);
// 	printf("</Condition>\n");



// 	const void *if_block = if_stmt_get_if_block(if_stmt);

// 	PRINT_SPACES(self);
// 	printf("<IfBlock>\n");
// 	self->tabs++;

// 	print_stmt(printer->stmt_visitor, if_block);

// 	if(self->tabs > 0) self->tabs--;

// 	PRINT_SPACES(self);
// 	printf("</IfBlock>\n");



// 	const void *else_block = if_stmt_get_else_block(if_stmt);

// 	if(NULL != else_block) {
// 		PRINT_SPACES(self);
// 		printf("<ElseBlock>\n");
// 		self->tabs++;

// 		print_stmt(printer->stmt_visitor, else_block);

// 		if(self->tabs > 0) self->tabs--;

// 		PRINT_SPACES(self);
// 		printf("</ElseBlock>\n");
// 	}


// 	if(self->tabs > 0) self->tabs--;

// 	PRINT_SPACES(self);
// 	printf("</IfStmt>\n");
// }



// static void print_when_stmt(const void *visitor, const void *when_stmt) {

// }






// /*
// loop(1 -> 10; 2) {
// 	print "hello";
// }

// loop(a < b) {
// 	print "a is less than b!";
// }


// LoopStmt {
// 	start_expr: LiteralExpr {
// 		value: Value { type: "INTEGER", value: "1" }
// 	}
// 	end_expr: LiteralExpr {
// 		value: Value { type: "INTEGER", value: "10" }
// 	}
// 	step_expr: LiteralExpr {
// 		value: Value { type: "INTEGER", value: "2" }
// 	}
// 	child_stmt: BlockStmt {
// 		statements: [
// 			PrintStmt {
// 				expr_to_print: LiteralExpr {
// 				value: Value { type: "STRING", value: "hello" }
// 				}
// 			}
// 		]
// 	}
// }


// <LoopStmt>

// 	<StartExpr>
// 		<LiteralExpr>
// 			<Value
// 				type="INTEGER"
// 				value="1" />
// 		</LiteralExpr>
// 	</StartExpr>

// 	<EndExpr>
// 		<LiteralExpr>
// 			<Value
// 				type="INTEGER"
// 				value="10" />
// 		</LiteralExpr>
// 	</EndExpr>

// 	<StepExpr>
// 		<LiteralExpr>
// 			<Value
// 				type="INTEGER"
// 				value="2" />
// 		</LiteralExpr>
// 	</StepExpr>

// 	<ChildStmt>
// 		<BlockStmt>
// 			<PrintStmt>
// 				<LiteralExpr>
// 					<Value
// 						type="STRING"
// 						value="hello" />
// 				</LiteralExpr>
// 			</PrintStmt>

// 			<PrintStmt>
// 				<LiteralExpr>
// 					<Value
// 						type="DOUBLE"
// 						value="3.14" />
// 				</LiteralExpr>
// 			</PrintStmt>
// 		</BlockStmt>
// 	</ChildStmt>

// </LoopStmt>
// */

// static void print_loop_stmt(const void *visitor, const void *loop_stmt) {
// 	struct AstPrinter *self = printer;
// 	assert(self && loop_stmt && (self->class == AstPrinter));

// }



// /*
// repeat(5 + 1) {
// 	print "hello";
// }

// <RepeatStmt>
// 	<Expression>
// 		<BinaryExpr operator="+">
// 			<LiteralExpr>
// 				<Value
// 					type="INTEGER"
// 					value="5" />
// 			</LiteralExpr>
// 			<LiteralExpr>
// 				<Value
// 					type="INTEGER"
// 					value="1" />
// 			</LiteralExpr>
// 		</BinaryExpr>
// 	</Expression>

// 	<ChildStmt>
// 		<BlockStmt>
// 			<PrintStmt>
// 				<LiteralExpr>
// 					<Value
// 						type="INTEGER"
// 						value="5" />
// 				</LiteralExpr>
// 			</PrintStmt>
// 		</BlockStmt>
// 	</ChildStmt>
// </RepeatStmt>
// */

// static void print_repeat_stmt(const void *visitor, const void *repeat_stmt) {
// 	struct AstPrinter *self = printer;
// 	assert(self && (self->class == AstPrinter));

// 	PRINT_SPACES(self);

// 	printf("<RepeatStmt>\n");
// 	self->tabs++;

// 	const void *expr = repeat_stmt_get_expr(repeat_stmt);

// 	PRINT_SPACES(self);
// 	printf("<Expr>\n");
// 	self->tabs++;

// 	print_expr(printer->expr_visitor, expr);

// 	if(self->tabs > 0) self->tabs--;

// 	PRINT_SPACES(self);
// 	printf("</Expr>\n");


// 	const void *child_stmt = repeat_stmt_get_child_stmt(repeat_stmt);

// 	PRINT_SPACES(self);
// 	printf("<ChildStmt>\n");
// 	self->tabs++;

// 	print_stmt(printer->stmt_visitor, child_stmt);

// 	if(self->tabs > 0) self->tabs--;

// 	PRINT_SPACES(self);
// 	printf("</ChildStmt>\n");


// 	if(self->tabs > 0) self->tabs--;

// 	PRINT_SPACES(self);
// 	printf("</RepeatStmt>\n");
// }



// static void print_block_stmt(const void *visitor, const void *block_stmt) {
// 	struct AstPrinter *self = printer;
// 	assert(self && (self->class == AstPrinter));

// 	PRINT_SPACES(self);

// 	printf("<BlockStmt>\n");
// 	self->tabs++;

// 	const void *const *child_stmts = block_stmt_get_child_stmts(block_stmt);

// 	for(int i=0; NULL != child_stmts[i]; i++) {
// 		print_stmt(printer->stmt_visitor, child_stmts[i]);
// 	}

// 	if(self->tabs > 0)
// 		self->tabs--;

// 	PRINT_SPACES(self);
// 	printf("</BlockStmt>\n");
// }








// static const void *print_identifier_expr(const void *visitor, const void *iexpr) {
	
// }



// static const void *print_literal_expr(const void *visitor, const void *expr) {
// 	struct AstPrinter *self = printer;
// 	assert(self && (self->class == AstPrinter));

// 	PRINT_SPACES(self);

// 	printf("<LiteralExpr>\n");
// 	self->tabs = self->tabs + 1;

// 	// print value of the literal expr
// 	PRINT_SPACES(self);

// 	printf("<Value");
// 	self->tabs = self->tabs + 1;

// 	const void *value = lexpr_get_value(expr);

// 	printf("\n");
// 	PRINT_SPACES(self);


// 	switch(value_get_value_type(value)) {
// 		case VALUE_TYPE_INTEGER: {
// 			printf("type=\"INTEGER\"\n");

// 			PRINT_SPACES(self);
// 			printf("value=\"%d\"", value_get_int_val(value));

// 			break;
// 		}
		
// 		case VALUE_TYPE_DOUBLE: {
// 			printf("type=\"DOUBLE\"\n");

// 			PRINT_SPACES(self);
// 			printf("value=\"%g\"", value_get_double_val(value));

// 			break;
// 		}
		
// 		case VALUE_TYPE_BOOL: {
// 			printf("type=\"BOOL\"\n");

// 			PRINT_SPACES(self);

// 			if(false == value_get_int_val(value))
// 				printf("value=\"false\"");

// 			else if(true == value_get_int_val(value))
// 				printf("value=\"true\"");

// 			break;
// 		}

// 		case VALUE_TYPE_STRING: {
// 			printf("type=\"STRING\"\n");

// 			PRINT_SPACES(self);
// 			printf("value=\"%s\"", (char *) value_get_obj_ptr(value));

// 			break;
// 		}
// 	}
// 	printf(" />\n");

// 	if(self->tabs - 1 > -1) {
// 		self->tabs = self->tabs - 1;
// 	}

// 	if(self->tabs - 1 > -1) {
// 		self->tabs = self->tabs - 1;
// 	}

// 	PRINT_SPACES(self);
// 	printf("</LiteralExpr>\n");

// 	return NULL;
// }



// static const void *print_unary_expr(const void *visitor, const void *uexpr) {
// 	struct AstPrinter *self = printer;
// 	assert(self && (self->class == AstPrinter));

// 	PRINT_SPACES(self);

// 	// open starting tag
// 	printf("<UnaryExpr");

// 	self->tabs = self->tabs + 1;


// 	// print attributes
// 	const struct Token *op_token = uexpr_get_operator(uexpr);

// 	printf(" ");
// 	// for(int i=0; i<self->tabs; i++) printf("\t");

// 	printf("%s=\"%.*s\"", "operator", op_token->length, op_token->start);


// 	// close starting tag
// 	printf(">\n");

// 	// print right child tag
// 	const void *right_expr = uexpr_get_right(uexpr);
// 	print_expr(self->expr_visitor, right_expr);


// 	if(self->tabs - 1 > -1) {
// 		self->tabs = self->tabs - 1;
// 	}

// 	PRINT_SPACES(self);
// 	printf("</UnaryExpr>\n");

// 	return NULL;
// }



// static const void *print_binary_expr(const void *visitor, const void *bexpr) {
// 	struct AstPrinter *self = printer;
// 	assert(self && (self->class == AstPrinter));

// 	PRINT_SPACES(self);

// 	// open starting tag
// 	printf("<BinaryExpr");

// 	self->tabs = self->tabs + 1;

// 	// print attributes
// 	const struct Token *op_token = bexpr_get_operator(bexpr);

// 	printf(" ");
// 	// for(int i=0; i<self->tabs; i++) printf("\t");

// 	printf("operator=\"%.*s\"", op_token->length, op_token->start);

// 	// close starting tag
// 	printf(">\n");

// 	// print left child tag
// 	const void *left_expr = bexpr_get_left_expr(bexpr);
// 	print_expr(self->expr_visitor, left_expr);

// 	// print left child tag
// 	const void *right_expr = bexpr_get_right_expr(bexpr);
// 	print_expr(self->expr_visitor, right_expr);

// 	if(self->tabs - 1 > -1) {
// 		self->tabs = self->tabs - 1;
// 	}

// 	PRINT_SPACES(self);
// 	printf("</BinaryExpr>\n");

// 	return NULL;
// }



// static const void *print_group_expr(const void *visitor, const void *gexpr) {
// 	struct AstPrinter *self = printer;
// 	assert(self && gexpr && (self->class == AstPrinter));

// 	PRINT_SPACES(self);
// 	printf("<GroupExpr>\n");
// 	self->tabs = self->tabs + 1;

// 	const void *child_expr = gexpr_get_child_expr(gexpr);
// 	print_expr(printer->expr_visitor, child_expr);

// 	if(self->tabs > 0) self->tabs--;

// 	PRINT_SPACES(self);
// 	printf("</GroupExpr>\n");
// }



// static const void *print_var_assign_expr(const void *visitor, const void *vaexpr) {
	
// }


// #undef PRINT_SPACES





// void print_ast(const void *_self) {
// 	struct AstPrinter *self = (void *)_self;
// 	assert(self && (self->class == AstPrinter));

// 	printer = self;

// 	for(uint32_t i = 0; NULL != printer->stmts[i]; i++)
// 		print_stmt(printer->stmt_visitor, printer->stmts[i]);
// }


