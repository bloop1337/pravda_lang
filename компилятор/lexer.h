#ifndef LEXER_H
#define LEXER_H 1
#include <stddef.h>
#include "token.h"
#include "./thirdparty/utf8.h"

void pravda_lexer_process(Pravda_Token_Stack *tokens, utf8_string code, const char *fpath);

#endif //LEXER_H
