#include <assert.h>

#define main _
	#include "прк.c"
#undef main

#define TEST(func) do {         \
	func();                     \
	printf("%s выполнен.\n", #func); \
} while (0); 

void test_lexer_case_0() {
	Token_Stack stack = {0};
	lexer_proc(&stack, make_utf8_string("42 + 67 - 69 * 1337 / 0;"), "тест0");
	Token_Type expected[] = {
		TOKEN_INT,
		TOKEN_PLUS,
		TOKEN_INT,
		TOKEN_MINUS,
		TOKEN_INT,
		TOKEN_ASTERISK,
		TOKEN_INT,
		TOKEN_SLASH,
		TOKEN_INT,
		TOKEN_SEMICOLON,
		TOKEN_EOF,
	};
	assert(stack.size == sizeof(expected) / sizeof(expected[0]));

	for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
		assert(stack.toks[i].type == expected[i]);
	}
	stack_free(&stack);
}

void test_lexer_case_1() {
	Token_Stack stack = {0};
	lexer_proc(&stack, make_utf8_string("пусть а: цел = 10;"), "тест0");
	Token_Type expected[] = {
 		TOKEN_KEYWORD_LET,
		TOKEN_IDENTIFIER,
		TOKEN_COLON,
		TOKEN_IDENTIFIER,
		TOKEN_ASSING,
		TOKEN_INT,
		TOKEN_SEMICOLON,
		TOKEN_EOF,
	};
	assert(stack.size == sizeof(expected) / sizeof(expected[0]));

	for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
		assert(stack.toks[i].type == expected[i]);
	}
	stack_free(&stack);
}

void test_parser_case_0() {
	Token_Stack stack = {0};
	utf8_string code = make_utf8_string("1;");
	lexer_proc(&stack, code, "тест0");
	Token_Type expected[] = { TOKEN_INT, TOKEN_SEMICOLON, TOKEN_EOF };
	for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i)
		assert(stack.toks[i].type == expected[i]);

	struct Ast_Node root = parser_proc(&stack, "тест", code);
	
	struct AST_BLOCK data = root.data.AST_BLOCK;
	printf("data.size = %zu\n", data.size);
	assert(data.block[0].kind == AST_NUMBER);
	assert(data.size == 1);
}

int 
main()
{	
	TEST(test_lexer_case_0);	
	TEST(test_lexer_case_1);	
	TEST(test_parser_case_0);	
	return 0;
}
