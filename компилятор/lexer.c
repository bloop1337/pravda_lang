#include "lexer.h"
#include "logger.h"
#include <stdio.h>
#include <ctype.h>
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
is_ident_char(utf8_char ch) {
	return (unicode_code_point(ch) >= 0x0410  && // А-Я
		unicode_code_point(ch) <= 0x044F) || 	// а-я
		unicode_code_point(ch) == 0x0401  || 	// Ё
		unicode_code_point(ch) == 0x0451  || 	// ё
		unicode_code_point(ch) == 0x005F;    	// _
}

static bool 
is_digit(utf8_char ch) 
{
	return isdigit(unicode_code_point(ch));
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
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("("), PRAVDA_LEFT_BRACKET, line, character});
			break;
		case 0x0029:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string(")"), PRAVDA_RIGHT_BRACKET, line, character});
			break;
		case 0x002A:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("*"), PRAVDA_ASTERISK, line, character});
			break;
		case 0x002B:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("+"), PRAVDA_PLUS, line, character});
			break;
		case 0x002D:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("-"), PRAVDA_MINUS, line, character});
			break;
		case 0x002F:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("/"), PRAVDA_SLASH, line, character});
			break;
		case 0x002C:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string(","), PRAVDA_COMMA, line, character});
			break;
		case 0x002E:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("."), PRAVDA_POINT, line, character});
			break;
		case 0x007B:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("{"), PRAVDA_LEFT_CURLY_BRACKET, line, character});
			break;
		case 0x007D:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("}"), PRAVDA_RIGHT_CURLY_BRACKET, line, character});
			break;
		case 0x003A:
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string(":"), PRAVDA_COLON, line, character});
			break;
		case 0x003C: {
			utf8_char next = peek_utf8_char(&iter);
			if (unicode_code_point(next) == 0x003D) {
				pravda_stack_push(stack, (Pravda_Token){make_utf8_string("<="), PRAVDA_LESS_OR_EQ_THAN, line, character});
				next_utf8_char(&iter);
				break;
			}
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("<"), PRAVDA_LESS_THAN, line, character});
			break;
		}
		case 0x003E: {
			utf8_char next = peek_utf8_char(&iter);
			if (unicode_code_point(next) == 0x003D) {
				pravda_stack_push(stack, (Pravda_Token){make_utf8_string(">="), PRAVDA_GREATER_OR_EQ_THAN, line, character});
				next_utf8_char(&iter);
				break;
			}
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string(">"), PRAVDA_GREATER_THAN, line, character});
			break;
		}
		case 0x003D: {
			utf8_char next = peek_utf8_char(&iter);
			if (unicode_code_point(next) == 0x003D) {
				pravda_stack_push(stack, (Pravda_Token){make_utf8_string("=="), PRAVDA_EQUAL, line, character});
				next_utf8_char(&iter);
				break;
			}
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("="), PRAVDA_ASSING, line, character});
			break;
		}
		case 0x0022: {
			pravda_stack_push(stack, (Pravda_Token){make_utf8_string("\""), PRAVDA_STRING, line, character});
			break;
		}
		case 0x000A: // new line
			line     += 1;
			character = 0;
		case 0x000B: // vertical tabulation
		case 0x0009: // tabulation
		case 0x000D: // return  
		case 0x0020: // space
			break;
		default:
			if (is_ident_char(ch)) { // А-Я а-я _
//				utf8_char next = peek_utf8_char(&iter);
//				while (is_ident_char(next)) {
//					// collect a identifier or keyword
//					ch   = next_utf8_char(&iter);
//					next = peek_utf8_char(&iter);
//				}
//				// push a identifier or keyword to the stack break;	}
			break;
			}
			if (is_digit(ch)) {
//				utf8_char next = peek_utf8_char(&iter);
//				while (is_digit(next)) {
//					// collect a number
//					ch   = next_utf8_char(&iter);
//					next = peek_utf8_char(&iter);
//				}
//				// push the number to the stack
				break;	
			}
			pravda_error_code(fpath, code, "Неизвестный символ", line, character);
			break;
	}	
	character += 1;
	}	
}
