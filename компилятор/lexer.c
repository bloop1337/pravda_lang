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

	printf("Stack was init. size = %zu, capc = %zu\n", stack->size, stack->capc);
	stack->tokens = malloc(32 * sizeof(Pravda_Token));
    stack->size   = 0;
    stack->capc   = 32;

	printf("Stack was init. size = %zu, capc = %zu\n", stack->size, stack->capc);
}

void
pravda_stack_push(Pravda_Token_Stack *stack, Pravda_Token token)
{
	printf("Begin push token into stack size = %zu, capc = %zu\n", stack->size, stack->capc);
	if (stack->capc <= stack->size) 
		stack->tokens = realloc(stack->tokens, (int)(stack->capc*=2) * sizeof(Pravda_Token));
	stack->size+=1;
	stack->tokens[stack->size-1] = token; 
	
	printf("Token was pushed in stack. size = %zu, capc = %zu\n", stack->size, stack->capc);
}




void 
pravda_lexer_process(Pravda_Token_Stack *stack,  utf8_string code) 
{

	printf("init stack\n");
	pravda_stack_init(stack);	

	utf8_char_iter iter = make_utf8_char_iter(code);
	
	printf("begin while loop\n");
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
		default:
			if (is_identifier(ch)) { // А-Я а-я 0-9 _
							 //
				break;	
			}

			if (is_digit(ch)) {
				break;
			}
			break;
	}	
	}
	
	printf("end while loop\n");
}
