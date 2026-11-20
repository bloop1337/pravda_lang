#ifndef PRAVDA_PARSER
#define PRAVDA_PARSER 1
#include "ast.h"

void pravda_parser_process(Pravda_Ast_Node *node, Pravda_Token_Stack *stack, utf8_string code, const char *fpath);

void pravda_parse_expression(Pravda_Ast_Node *node, Pravda_Token_Stack *stack, utf8_string code, const char *fpath);
void pravda_parse_function_def(Pravda_Ast_Node *node, Pravda_Token_Stack *stack, utf8_string code, const char *fpath);
void pravda_parse_variable_def(Pravda_Ast_Node *node, Pravda_Token_Stack *stack, utf8_string code, const char *fpath);

#endif
