#ifndef PRAVDA_TOKEN
#define PRAVDA_TOKEN 1
#include "./thirdparty/utf8.h"

/* порядок ключ. слов не менять, это важно для лексера. */
typedef enum {
	PRAVDA_KEYWORD_FN,         // фн
	PRAVDA_KEYWORD_RETURN,     // верни 
	PRAVDA_KEYWORD_END,        // конец
	PRAVDA_KEYWORD_LET,        // пусть
	PRAVDA_IDENTIFIER,         // названия 
	PRAVDA_INT,                // 2012313
	PRAVDA_LEFT_BRACKET,       // (
	PRAVDA_RIGHT_BRACKET,      // )
	PRAVDA_POINT,              // .
	PRAVDA_STRING,             // "*"
	PRAVDA_ASSING,             // =
	PRAVDA_COLON               // :
} Pravda_Token_Type;

typedef struct {
	size_t line;
	size_t colm;
} Pravda_Code_Position;

typedef struct {
	owned_utf8_string content; // content.str = NULL if type != IDENTIFIER OR KEYWORD OR STRING
	Pravda_Token_Type type;
	Pravda_Code_Position pos;
} Pravda_Token;

typedef struct {
	Pravda_Token *tokens;
	size_t size;
	size_t capc;
} Pravda_Token_Stack;

void pravda_stack_init(Pravda_Token_Stack *stack);
void pravda_stack_push(Pravda_Token_Stack *stack, Pravda_Token token);

void pravda_free_token(Pravda_Token *token);

#endif
