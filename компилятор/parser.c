#include "parser.h"
#include "logger.h"

void
pravda_parse_function_def(Pravda_Ast_Node *node, Pravda_Token_Stack *stack, size_t index, utf8_string code, const char *fpath)
{
	/*
	 *
	 * из:
	 *	фн главная(): цел
	 *  	верни 0.
	 *	конец
	 * в:
	 *  объявление функции 
	 *  	- возвращаемое значение: целое
	 *  	- идентификатор: главная
	 *  	- параметры: 
	 *  	- тело функции
	 *  		- вернуть
	 *  			- константа: 0 
	 */
	Pravda_Token_Type expected = PRAVDA_KEYWORD_FN;
	Pravda_Token_Type next_exp = PRAVDA_IDENTIFIER;

	for (size_t i = index; i < stack->size; ++i) {
		Pravda_Token *type = &stack->tokens[i];
		
	}
}

void
pravda_parse_variable_def(Pravda_Ast_Node *node, Pravda_Token_Stack *stack, utf8_string code, const char *fpath)
{

}

void
pravda_parser_process(Pravda_Ast_Node *root, Pravda_Token_Stack *stack, utf8_string code, const char *path)
{
	for (size_t i = 0; i < stack->size; ++i) {
		Pravda_Token *token = &stack->tokens[i];

		switch (token->type) {
			case PRAVDA_KEYWORD_FN:
				pravda_parse_function_def(root, stack, code, path);
				break;
			case PRAVDA_KEYWORD_LET:					
				pravda_parse_variable_def(root, stack, code, path);
				break;
			default:
				pravda_error_code(
					path, code, "Неизвестное слово",
					token->pos.line, token->pos.colm
				);
				break;
		}
	}
}
