#ifndef PRAVDA_AST
#define PRAVDA_AST 1

typedef struct {
	Pravda_Token* token;
	Pravda_Ast_Node* child;
} Pravda_Ast_Node;

#endif
