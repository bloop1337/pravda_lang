#include "lexer.h"
#include "logger.h"
#include "token.h"
#include <string.h>
#include <ctype.h>
#include "./thirdparty/utf8.h"

static char *keywords[] = {
	"фн",
	"верни",
	"конец",
	"пусть",
};

/* Возвращает номер Token_Type если это ключевое слово. Иначе -1 */
static unsigned int 
pravda_is_keyword(const char* s) 
{
	size_t n = sizeof(keywords) / sizeof(keywords[0]);
	for (size_t i = 0; i < n; ++i) { 
		if (strcmp(keywords[i], s) == 0) { 
			return i; 
		}
	}
	return -1;
}

static bool 
is_ident_char(utf8_char ch)
{
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

static Pravda_Token
pravda_create_token(Pravda_Token_Type type, int line, int character)
{
	return (Pravda_Token) { .content = empty_owned_string(), .type = type, (Pravda_Code_Position) {line, character} };
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
		case 0x002E:
			pravda_stack_push(stack, pravda_create_token(PRAVDA_POINT, line, character));
			break;
		case 0x003A:
			pravda_stack_push(stack, pravda_create_token(PRAVDA_COLON, line, character));
			break;
		case 0x003D: {
			pravda_stack_push(stack, pravda_create_token(PRAVDA_ASSING, line, character));
			break;
		}
		case 0x0022: {
			utf8_char next = peek_utf8_char(&iter);	
			if (unicode_code_point(next) == 0x0022) {
				pravda_stack_push(stack, pravda_create_token(PRAVDA_STRING, line, character)); // EMPTY STR
				ch = next_utf8_char(&iter); // TAKE "
				character++;
				break;
			}

			owned_utf8_string str = owned_string_with_capacity(20);
			while (((next = peek_utf8_char(&iter)).byte_len > 0) && unicode_code_point(next) != 0x0022) {
				ch = next_utf8_char(&iter);
				character++;
				owned_string_push_back(&str, ch);
			}
			pravda_stack_push(stack, (Pravda_Token) { .content = str, .type = PRAVDA_STRING, .pos = (Pravda_Code_Position) { .line = line, .colm = character} });
			ch = next_utf8_char(&iter); // TAKE "
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
					character++;
				}
				int keyword_type = pravda_is_keyword(str.str);
				if (keyword_type < 0) {
					/* воспринимаем как идентификатор */
					pravda_stack_push(stack, (Pravda_Token) { .content = str, .type = PRAVDA_IDENTIFIER, .pos = (Pravda_Code_Position) { .line = line, .colm = character} });
				} else {
					/* воспринимаем как ключ. слово*/
					pravda_stack_push(stack, (Pravda_Token) { .content = str, .type = (Pravda_Token_Type)keyword_type, .pos = (Pravda_Code_Position) { .line = line, .colm = character} });
				}

				break;
			}
			if (is_digit(ch)) {
				owned_utf8_string str = owned_string_with_capacity(6);
				utf8_char next;

				owned_string_push_back(&str, ch);
				while ((next = peek_utf8_char(&iter)).byte_len > 0 && is_digit(next)) {
					ch = next_utf8_char(&iter);
					owned_string_push_back(&str, ch);	
					character++;
				}

				pravda_stack_push(stack, (Pravda_Token) { .content = str, .type = PRAVDA_INT, .pos = (Pravda_Code_Position) { .line = line, .colm = character} });
				break;
			}
			pravda_error_code(fpath, code, "Неизвестный символ", line, character);
			break;
	}	
	character += 1;
	}	
}
