#include "lexer.h"
#include "logger.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static const char* keywords[] = {
	"фн", "верни", "конец", "подключи"
}; 

static bool pravda_is_keyword(const char* s) {
	size_t n = sizeof(keywords) / sizeof(keywords[0]);
	for (size_t i = 0; i < n; ++i) if (strcmp(keywords[i], s) == 0) return true;
	return false;
}

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

static Pravda_Token
pravda_create_token(Token_Type type, int line, int character)
{
	return (Pravda_Token) { .content = empty_owned_string(), .type = type, (Pravda_Code_Postion) {line, character} };
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
			pravda_stack_push(stack, pravda_create_token(PRAVDA_LEFT_BRACKET, line, character));
			break;
		case 0x0029:
			pravda_stack_push(stack, pravda_create_token(PRAVDA_RIGHT_BRACKET, line, character));
			break;
		case 0x002A:
			pravda_stack_push(stack, pravda_create_token(PRAVDA_ASTERISK, line, character));
			break;
		case 0x002B:
			pravda_stack_push(stack, pravda_create_token(PRAVDA_ASTERISK, line, character));
			break;
		case 0x002D:
			pravda_stack_push(stack, pravda_create_token(PRAVDA_MINUS, line, character));
			break;
		case 0x002F:
			pravda_stack_push(stack, pravda_create_token(PRAVDA_SLASH, line, character));
			break;
		case 0x002C:
			pravda_stack_push(stack, pravda_create_token(PRAVDA_COMMA, line, character));
			break;
		case 0x002E:
			pravda_stack_push(stack, pravda_create_token(PRAVDA_POINT, line, character));
			break;
		case 0x007B:
			pravda_stack_push(stack, pravda_create_token(PRAVDA_LEFT_CURLY_BRACKET, line, character));
			break;
		case 0x007D:
			pravda_stack_push(stack, pravda_create_token(PRAVDA_RIGHT_CURLY_BRACKET, line, character));
			break;
		case 0x003A:
			pravda_stack_push(stack, pravda_create_token(PRAVDA_COLON, line, character));
			break;
		case 0x003C: {
			utf8_char next = peek_utf8_char(&iter);
			if (unicode_code_point(next) == 0x003D) {
				pravda_stack_push(stack, pravda_create_token(PRAVDA_LESS_OR_EQ_THAN, line, character));
				next_utf8_char(&iter);
				break;
			}
			pravda_stack_push(stack, pravda_create_token(PRAVDA_LESS_THAN, line, character));
			break;
		}
		case 0x003E: {
			utf8_char next = peek_utf8_char(&iter);
			if (unicode_code_point(next) == 0x003D) {
				pravda_stack_push(stack, pravda_create_token(PRAVDA_GREATER_OR_EQ_THAN, line, character));
				ch = next_utf8_char(&iter);
				break;
			}
			pravda_stack_push(stack, pravda_create_token(PRAVDA_GREATER_THAN, line, character));
			break;
		}
		case 0x003D: {
			utf8_char next = peek_utf8_char(&iter);
			if (unicode_code_point(next) == 0x003D) {
				pravda_stack_push(stack, pravda_create_token(PRAVDA_EQUAL, line, character));
				ch = next_utf8_char(&iter);
				break;
			}
			pravda_stack_push(stack, pravda_create_token(PRAVDA_ASSING, line, character));
			break;
		}
		case 0x0022: {
			utf8_char next = peek_utf8_char(&iter);	
			if (unicode_code_point(next) == 0x0022) {
				pravda_stack_push(stack, pravda_create_token(PRAVDA_STRING, line, character));
				ch = next_utf8_char(&iter);
				break;
			}

			owned_utf8_string str = owned_string_with_capacity(20);
			while (((next = peek_utf8_char(&iter)).byte_len > 0) && unicode_code_point(next) != 0x0022) {
				ch = next_utf8_char(&iter);
				owned_string_push_back(&str, ch);
			}
			pravda_stack_push(stack, (Pravda_Token) { .content = str, .type = PRAVDA_STRING, .pos = (Pravda_Code_Postion) { .line = line, .colm = character} });
			ch = next_utf8_char(&iter);
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
			if (is_ident_char(ch)) {
				owned_utf8_string str = owned_string_with_capacity(20);
				utf8_char next;

				owned_string_push_back(&str, ch);
				while ((next = peek_utf8_char(&iter)).byte_len > 0 && (is_ident_char(next) || is_digit(next))) {
					ch = next_utf8_char(&iter);
					owned_string_push_back(&str, ch);	
				}
				if (pravda_is_keyword(str.str)) {
					pravda_stack_push(stack, (Pravda_Token) { .content = str, .type = PRAVDA_KEYWORD, .pos = (Pravda_Code_Postion) { .line = line, .colm = character} });
				} else {
					pravda_stack_push(stack, (Pravda_Token) { .content = str, .type = PRAVDA_IDENTIFIER, .pos = (Pravda_Code_Postion) { .line = line, .colm = character} });
				}

				break;
			}
			pravda_error_code(fpath, code, "Неизвестный символ", line, character);
			break;
	}	
	character += 1;
	}	
}
