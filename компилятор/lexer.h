#ifndef LEXER_H
#define LEXER_H 1
#include <stddef.h>
#include "./thirdparty/utf8.h"

#ifndef PRAVDA_TOKEN_STACK_START_SIZE
#define PRAVDA_TOKEN_STACK_START_SIZE 32
#endif // TOKEN_STACK_START_SIZE

typedef enum {
	PRAVDA_KEYWORD,	           // фн, включи, пока, если...
	PRAVDA_IDENTIFIER,          // названия и тд
	PRAVDA_INT,                // 2012313
	PRAVDA_FLOAT,              // 0.301230
	PRAVDA_SIMECOLON,          // ;
	PRAVDA_LEFT_BRACKET,       // (
	PRAVDA_RIGHT_BRACKET,      // )
	PRAVDA_LEFT_CURLY_BRACKET, // {
	PRAVDA_RIGHT_CURLY_BRACKET,// {
	PRAVDA_PLUS,		       // +
	PRAVDA_MINUS,              // - 
	PRAVDA_SLASH,              // /
	PRAVDA_ASTERISK,           // *
	PRAVDA_COMMA,              // ,
	PRAVDA_POINT,              // .
	PRAVDA_STRING,             // "*"
	PRAVDA_ASSING,             // =
	PRAVDA_EQUAL,              // ==
	PRAVDA_GREATER_THAN,       // >
	PRAVDA_LESS_THAN,          // <
	PRAVDA_LESS_OR_EQ_THAN,    // <=
	PRAVDA_GREATER_OR_EQ_THAN, // >=
} Token_Type;

typedef struct {
	utf8_string content;
	Token_Type type;
} Pravda_Token;

typedef struct {
	Pravda_Token *tokens;
	size_t size;
	size_t capc;
} Pravda_Token_Stack;

void pravda_stack_init(Pravda_Token_Stack *stack);
void pravda_stack_push(Pravda_Token_Stack *stack, Pravda_Token token);

void pravda_lexer_process(Pravda_Token_Stack *tokens, utf8_string code);

#endif //LEXER_H
