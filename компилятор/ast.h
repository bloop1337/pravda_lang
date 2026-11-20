#ifndef PRAVDA_AST
#define PRAVDA_AST 1

#include "token.h"

typedef struct Pravda_Ast_Node Pravda_Ast_Node;

struct Pravda_Ast_Node {
	Pravda_Token token;
	Pravda_Ast_Node *nodes;
	size_t size;
	size_t capc;
};

Pravda_Ast_Node pravda_node_with_capacity(size_t capacity);
void pravda_node_push_back(Pravda_Ast_Node *parent, Pravda_Ast_Node *child);
void pravda_node_free(Pravda_Ast_Node* node);

#endif
