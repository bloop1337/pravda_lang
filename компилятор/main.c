#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lexer.h"
// цель: написать минимальный компилятор,
// чтоб начать писать компилятор на правде
int main(int argc, char** argv) 
{	
	Pravda_Token *tokens = NULL;
	size_t tokens_length = 0;
	pravda_lexer_process(tokens, &tokens_length, "");
	
	for (size_t i = 0; i < tokens_length; ++i)
		printf("%s", tokens[i].content);

	return 0;
}
