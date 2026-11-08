#ifndef LEXER_H
#define LEXER_H
#include <stddef.h>

typedef enum {
	PRAVDA_KEYWORD,	           // фн, включи, пока, если...
	PRAVDA_INT,	               // 2012313
	PRAVDA_FLOAT,              // 0.301230
	PRAVDA_SIMECOLON,          // ;
	PRAVDA_LEFT_BRACKET,       // (
	PRAVDA_RIGHT_BRACKET,      // )
	PRAVDA_PLUS,		       // +
	PRAVDA_MINUS,              // - 
	PRAVDA_SLASH,              // /
	PRAVDA_ASTERISK,           // *
	PRAVDA_COMMENT,            // %
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
	const char *content;
	const Token_Type type;
} Pravda_Token;

void pravda_lexer_process(Pravda_Token *tokens, size_t *length, const char *code);

#endif //LEXER_H
