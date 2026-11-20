#include <stdlib.h>

#include "ast.h"

Pravda_Ast_Node
pravda_node_with_capacity(size_t cap)
{
	Pravda_Ast_Node *tmp = malloc(sizeof(Pravda_Ast_Node) * cap);
	if (!tmp) return (Pravda_Ast_Node) {0};
	
	Pravda_Ast_Node result = {0};
	result.nodes = tmp;
	result.capc  = cap;

	return result;
}

void
pravda_node_push_back(Pravda_Ast_Node *parent, Pravda_Ast_Node *child)
{
	if (parent->size == parent->capc) {
		Pravda_Ast_Node *tmp = realloc(parent->nodes, parent->capc*2);
		if (!tmp) return;
		parent->nodes = tmp;
		parent->capc  = parent->capc*2;
	}

	parent->nodes[parent->size] = *child;
	parent->size += 1;
}

void 
pravda_node_free(Pravda_Ast_Node* node)
{
	/* TODO */
}

