#include "token.h"
#include "stdlib.h"

#ifndef PRAVDA_TOKEN_STACK_START_SIZE
#define PRAVDA_TOKEN_STACK_START_SIZE 32
#endif // TOKEN_STACK_START_SIZE

void 
pravda_stack_init(Pravda_Token_Stack *stack) 
{
	stack->tokens = malloc(PRAVDA_TOKEN_STACK_START_SIZE * sizeof(Pravda_Token)); // и так сойдет
	stack->size   = 0;
	stack->capc   = PRAVDA_TOKEN_STACK_START_SIZE;
}

void
pravda_stack_push(Pravda_Token_Stack *stack, Pravda_Token token)
{
	if (stack->capc <= stack->size) {
		Pravda_Token *temp = realloc(stack->tokens, (int)(stack->capc*2) * sizeof(Pravda_Token));
		if (!temp) return; // кинуть ошибку? и так сойдет
		stack->tokens = temp;
		stack->capc = stack->capc * 2;
	}
	stack->size+=1;
	stack->tokens[stack->size-1] = token; 	
}

void
pravda_free_token(Pravda_Token *token)
{
	if (token->content.str != NULL) free_owned_utf8_string(&token->content);
	token->pos = (Pravda_Code_Position){0};
	token->type = 0;
}

