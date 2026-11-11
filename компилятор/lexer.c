#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>

static const char* keywords[] = {
	"фн",       "если", 
	"либо",     "иначе",
	"пока",     "верни",
	"конст",    "сломай",
	"пропусти", "прыгни",
	"структ",   "или", 
	"и",        "не"
}; 

static bool 
is_identifier(utf8_char ch) {
	return (unicode_code_point(ch) >= 0x0410  && // А-Я
		unicode_code_point(ch) <= 0x044F) || 	// а-я
		unicode_code_point(ch) == 0x0401  || 	// Ё
		unicode_code_point(ch) == 0x0451  || 	// ё
		unicode_code_point(ch) == 0x005F;    	// _
}

static bool 
is_digit(utf8_char ch) 
{
	return (unicode_code_point(ch) >= 0x0030  && // 0 - 9
		unicode_code_point(ch) <= 0x0039);   // 0 - 9
}

void 
pravda_stack_init(Pravda_Token_Stack *stack) 
{
	stack->tokens = malloc(32 * sizeof(Pravda_Token));
    stack->size   = 0;
    stack->capc   = 32;
}

void
pravda_stack_push(Pravda_Token_Stack *stack, Pravda_Token token)
{
	if (stack->capc <= stack->size) 
		stack->tokens = realloc(stack->tokens, (int)(stack->capc*=2) * sizeof(Pravda_Token));
	stack->size+=1;
	stack->tokens[stack->size-1] = token; 
	
}




void 
pravda_lexer_process(Pravda_Token_Stack *stack, utf8_string code, const char *fpath) 
{
	pravda_stack_init(stack);
	
	int line      = 1;
	int character = 1;

	utf8_char_iter iter = make_utf8_char_iter(code);
	utf8_char ch;
	while ((ch = next_utf8_char(&iter)).byte_len > 0) {
	switch (unicode_code_point(ch)) {
		case 0x0028:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("("), PRAVDA_LEFT_BRACKET});
			break;
		case 0x0029:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string(")"), PRAVDA_RIGHT_BRACKET});
			break;
		case 0x002A:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("*"), PRAVDA_ASTERISK});
			break;
		case 0x002B:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("+"), PRAVDA_PLUS});
			break;
		case 0x002D:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("-"), PRAVDA_MINUS});
			break;
		case 0x002F:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("/"), PRAVDA_SLASH});
			break;
		case 0x002C:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string(","), PRAVDA_COMMA});
			break;
		case 0x002E:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("."), PRAVDA_POINT});
			break;
		case 0x007B:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("{"), PRAVDA_LEFT_CURLY_BRACKET});
			break;
		case 0x007D:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("}"), PRAVDA_RIGHT_CURLY_BRACKET});
			break;
		case 0x003B:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string(";"), PRAVDA_SEMICOLON});
			break;
		case 0x003A:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string(":"), PRAVDA_COLON});
			break;
		case 0x003C: // TODO: <= case
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("<"), PRAVDA_LESS_THAN});
			break;
		case 0x003E: // TODO >= case
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string(">"), PRAVDA_GREATER_THAN});
			break;
		case 0x003D: // TODO == case
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("="), PRAVDA_ASSING});
			break;
		case 0x000A: // new line
			line     += 1;
			character = 0;
		case 0x000B: // vertical tabulation
		case 0x0009: // tabulation
		case 0x000D: // return  
		case 0x0020: // space
			break;
		default:
			if (is_identifier(ch)) { // А-Я а-я _
				break;	
			}

			if (is_digit(ch)) {
				break;
			}
			pravda_lexer_error(fpath, code, "Unexpected caracter", line, character);
			break;
	}	
	character += 1;
	}
	
}


//void
//pravda_lexer_error(const char *fpath, utf8_string code, const char *what, int line, int character)
//{
//	printf("%s:%i:%i error: %s\n", fpath, line, character, what);
//}

void
pravda_lexer_error(const char *fpath, utf8_string code, const char *what, int line, int character)
{
	const char *rst  = "\033[0m";
	const char *bld  = "\033[1m";
	const char *red  = "\033[1;31m";

	fprintf(stderr, "%s%s:%i:%i%s %serror%s: %s%s%s\n",
		bld, fpath, line, character, rst, red, rst,	bld, what, rst);
}
