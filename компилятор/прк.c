#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/stat.h>

#define UTF8_IMPL
#include "ютф8.h"

#define AST_NODE_NEW(tag, ...) \
	ast_node_new((struct Ast_Node){ tag, {.tag = (struct tag){ __VA_ARGS__ }} })

/* enums */
typedef enum {
	TOKEN_KEYWORD_FN,         /* фн       */      
	TOKEN_KEYWORD_RETURN,     /* верни    */
	TOKEN_KEYWORD_IF,         /* если     */
	TOKEN_KEYWORD_WHILE,      /* пока     */
	TOKEN_KEYWORD_END,        /* конец    */
	TOKEN_KEYWORD_LET,        /* пусть    */
	TOKEN_IDENTIFIER,         /* названия */ 
	TOKEN_INT,                /* 2012313  */
	TOKEN_LEFT_BRACKET,       /* (        */
	TOKEN_RIGHT_BRACKET,      /* )        */
	TOKEN_SEMICOLON,          /* ;        */
	TOKEN_STRING,             /* "*"      */
	TOKEN_COMMA,              /* ","      */
	TOKEN_ASSING,             /* =        */
	TOKEN_COLON,              /* :        */
	TOKEN_PLUS,               /* +        */
	TOKEN_MINUS,              /* -        */
	TOKEN_ASTERISK,           /* *        */
	TOKEN_SLASH,              /* /        */
	TOKEN_EOF                 /* End      */
} Token_Type;

/*structs*/
typedef struct {
	Token_Type type;
	owned_utf8_string content;

	size_t line;
	size_t colm;
} Code_Token;

typedef struct {
	Code_Token *toks;
	size_t size;
	size_t capc;
} Token_Stack;

typedef struct {
	Code_Token *current;
} Token_Stack_Iter;

typedef struct {
	utf8_string str;
	unsigned tabs;
} Code_Line;

typedef struct {
	const char *path;
	utf8_string code;

	size_t line;
	size_t colm;
} Err_Info;


/*
 * долго не мог разобратся как реализовать АСТ на си, но нашел
 * очень полезную статью, если интересно, рекомендую к прочтению
 * https://keleshev.com/abstract-syntax-tree-an-example-in-c/
 */
struct Ast_Node {
	enum {
		AST_BLOCK,
		AST_NUMBER,
		AST_PLUS,
		AST_MINUS,
		AST_MULTIPLE,
		AST_DEVIDE,
		AST_TYPE,
		AST_VAR_DEF,
	} kind;
	union {
		struct AST_BLOCK{
			struct Ast_Node *block;
			size_t size;
			size_t capc;
		} AST_BLOCK;
		struct AST_NUMBER { 
			int value;
		} AST_NUMBER;
		struct AST_PLUS     {
			struct Ast_Node *lhs;
			struct Ast_Node *rhs;
		} AST_PLUS;
		struct AST_MINUS    {
			struct Ast_Node *lhs; 
			struct Ast_Node *rhs;
		} AST_MINUS;
		struct AST_MULTIPLE {
			struct Ast_Node *lhs; 
			struct Ast_Node *rhs;
		} AST_MULTIPLE;
		struct AST_DEVIDE   {
			struct Ast_Node *lhs; 
			struct Ast_Node *rhs;
		} AST_DEVIDE;
		struct AST_TYPE     {
			enum {
				TYPE_UINT_8,
				TYPE_UINT_16,
				TYPE_UINT_32,
				TYPE_INT_8,
				TYPE_INT_16,
				TYPE_INT_32,
			} type;
		} AST_TYPE;
		struct AST_VAR_DEF  {
			owned_utf8_string id;
			struct Ast_Node *type;
			struct Ast_Node *expr;
		} AST_VAR_DEF;
	} data;
};

/*functions decl*/
static char *read_file(const char *name);
static int is_file(const char *path);

static Code_Token create_token(Token_Type type, int line, int colm);

static void stack_init(Token_Stack *stack, size_t capacity);
static void stack_push(Token_Stack *stack, Code_Token token);
static void stack_free(Token_Stack *stack);

static Token_Stack_Iter stack_iter(Token_Stack *stack);
static int stack_iter_has_next(Token_Stack_Iter *iter);
static void stack_iter_next(Token_Stack_Iter *iter);
static Code_Token* stack_iter_peek(Token_Stack_Iter *iter);
static void stack_iter_check(Token_Stack_Iter *iter, Token_Type type, const char *path, utf8_string code);
static void stack_iter_accept(Token_Stack_Iter *iter, Token_Type type, const char *path, utf8_string code);

static unsigned int is_keyword(const char* s);
static int  is_ident_char(utf8_char ch);
static void lexer_proc(Token_Stack *tokens, utf8_string code, const char* fpath);

static void error_exit(const char *what, int error_code);
static void error_code(const char *what, Err_Info info);

static void print_token(Code_Token *token);

static int is_operator(Token_Type type);
static struct Ast_Node *new_node_from_operator(struct Ast_Node *lhs, struct Ast_Node *rhs, Token_Type op);

static struct Ast_Node *ast_node_new(struct Ast_Node node); 
static void ast_node_block(struct Ast_Node *root, size_t capacity);
static void ast_node_block_push_back(struct Ast_Node *root, struct Ast_Node *node);

static void ast_node_print(struct Ast_Node *node, int level);

static struct Ast_Node parser_proc(Token_Stack *tokens, const char *path, utf8_string code);

static struct Ast_Node *parser_parse_let(Token_Stack_Iter *iter, const char *path, utf8_string code);
static struct Ast_Node *parser_parse_type(Token_Stack_Iter *iter, const char *path, utf8_string code);
static struct Ast_Node *parser_parse_expr(int min_bp, Token_Stack_Iter *iter, const char *path, utf8_string code);

static int l_binding_power(Token_Type type);
static int r_binding_power(Token_Type type);

/*constants*/
static char *keywords[] = { "фн", "верни", "если", "пока", "конец", "пусть", };
static const char *Type_Names[] = {
	[TYPE_UINT_8]  = "нат8",
	[TYPE_UINT_16] = "нат16",
	[TYPE_UINT_32] = "нат32",
	[TYPE_INT_8]   = "цел8",
	[TYPE_INT_16]  = "цел16",
	[TYPE_INT_32]  = "цел32",
};

static const char *Token_Names[] = {
	[TOKEN_KEYWORD_FN]          = "KEYWORD_FN",
	[TOKEN_KEYWORD_RETURN]      = "KEYWORD_RETURN",
	[TOKEN_KEYWORD_IF]          = "KEYWORD_IF",
	[TOKEN_KEYWORD_WHILE]       = "KEYWORD_WHILE",
	[TOKEN_KEYWORD_END]         = "KEYWORD_END",
	[TOKEN_KEYWORD_LET]         = "KEYWORD_LET",
	[TOKEN_IDENTIFIER]          = "IDENTIFIER",
	[TOKEN_INT]                 = "INT",
	[TOKEN_LEFT_BRACKET]        = "LEFT_BRACKET",
	[TOKEN_RIGHT_BRACKET]       = "RIGHT_BRACKET",
	[TOKEN_SEMICOLON]           = "SEMICOLON",
	[TOKEN_STRING]              = "STRING",
	[TOKEN_ASSING]              = "ASSING",
	[TOKEN_COLON]  			    = "COLON",
	[TOKEN_COMMA]  			    = "COMMA",
	[TOKEN_ASTERISK]  			= "ASTERISK",
	[TOKEN_SLASH]  			    = "SLASH",
	[TOKEN_PLUS]  			    = "PLUS",
	[TOKEN_MINUS]  			    = "MINUS",
	[TOKEN_EOF]  			    = "END OF FILE",
};

/*function impl*/
static void
stack_init(Token_Stack *stack, size_t capacity) 
{
	Code_Token *temp = malloc(capacity * sizeof(Code_Token));
	if (!temp) return; // ошибка алокации( 

	stack->toks = temp;
	stack->size   = 0;
	stack->capc   = capacity;
}

static void
stack_push(Token_Stack *stack, Code_Token token)
{
	if (stack->capc <= stack->size) {
		Code_Token *temp = realloc(stack->toks, (int)(stack->capc*2) * sizeof(Code_Token));
		if (!temp) return; // ошибка реалокации

		stack->toks = temp;
		stack->capc = stack->capc * 2;
	}
	stack->size+=1;
	stack->toks[stack->size-1] = token; 	
}

static void
stack_free(Token_Stack *stack)
{
	free(stack->toks);
	stack->capc   = 0;
	stack->size   = 0;
}

static Token_Stack_Iter
stack_iter(Token_Stack *stack)
{
	return (Token_Stack_Iter) { .current = stack->toks };
}

static int
stack_iter_has_next(Token_Stack_Iter *iter)
{
	return !(iter->current->type == TOKEN_EOF);
}

static void
stack_iter_next(Token_Stack_Iter *iter)
{
	if (stack_iter_has_next(iter)) iter->current++;
}

static Code_Token* 
stack_iter_peek(Token_Stack_Iter *iter)
{
	Code_Token *cur = iter->current;
	stack_iter_next(iter);
	Code_Token *next = iter->current;
	iter->current = cur;
	return next;
}
static void
stack_iter_check(Token_Stack_Iter *iter, Token_Type type, const char *path, utf8_string code)
{
	if (iter->current->type == type) return;
	error_code("Неожиданный токен: {}! Ожидался: {}", (Err_Info) { path, code, iter->current->line, iter->current->colm });
}

static void
stack_iter_accept(Token_Stack_Iter *iter, Token_Type type, const char *path, utf8_string code)
{
	if (iter->current->type == type) {
		stack_iter_next(iter);
		return;
	}

	error_code("Неожиданный токен: {}! Ожидался: {}", (Err_Info) { path, code, iter->current->line, iter->current->colm });
}

static void
stack_iter_consume(Token_Stack_Iter *iter, Token_Type type, const char *path, utf8_string code)
{
	if (iter->current->type == type) {
		stack_iter_next(iter);
		return;
	}

	// TODO: как то внедрить тип токена в диагностику
	error_code("Неожиданный токен: {}! Ожидался: {}", (Err_Info){path, code, iter->current->line, iter->current->colm });
}

/* Возвращает номер Token_Type если это ключевое слово. Иначе -1 */
static unsigned int 
is_keyword(const char* s) 
{
	size_t n = sizeof(keywords) / sizeof(keywords[0]);
	for (size_t i = 0; i < n; ++i) { 
		if (strcmp(keywords[i], s) == 0) { 
			return i; 
		}
	}
	return -1;
}

static int 
is_ident_char(utf8_char ch)
{
	return (unicode_code_point(ch) >= 0x0410  && // А-Я
		unicode_code_point(ch) <= 0x044F) || 	// а-я
		unicode_code_point(ch) == 0x0401  || 	// Ё
		unicode_code_point(ch) == 0x0451  || 	// ё
		unicode_code_point(ch) == 0x005F;    	// _
}

static Code_Token
create_token(Token_Type type, int line, int colm)
{
	return (Code_Token) { .content = empty_owned_string(), .type = type, .line = line, .colm = colm };
}

static Code_Line
code_line(utf8_string code, int line) {
    utf8_char_iter iter = make_utf8_char_iter(code);
    utf8_char ch;
    int current_line = 1;
    size_t start_index = 0;
	int tabs = 0;

    while (current_line < line) {
        ch = next_utf8_char(&iter);
        if (ch.byte_len == 0) return (Code_Line){ make_utf8_string(""), tabs };
        if (unicode_code_point(ch) == 0x000A) current_line++;
        start_index += ch.byte_len;
    }

    size_t end_index = start_index;
    while ((ch = next_utf8_char(&iter)).byte_len > 0) {
        if (unicode_code_point(ch) == 0x000A) break;
		if (unicode_code_point(ch) == 0x0009) {
			tabs++;
			start_index++;
		}
        end_index += ch.byte_len;
    }
    return (Code_Line) { slice_utf8_string(code, start_index, end_index - start_index), tabs };
}

static void
error_exit(const char *what, int exit_code)
{
	fprintf(stderr, "\x1B[%imошибка:\x1B[%im \x1B[%im%s\x1B[%im\n", 31, 0, 1, what, 0);
	exit(exit_code);
}

static void
error_code(const char* what, Err_Info info)
{
	fprintf(stderr, "\x1B[%im%s:%zu:%zu \x1B[%imошибка:\x1B[%im \x1B[%im%s\x1B[%im\n|\n", 1, info.path, info.line, info.colm, 31, 0, 1, what, 0);

	Code_Line problem_code = code_line(info.code, info.line);

	fprintf(stderr, "| %*zu: %.*s\n", 6, info.line, (int)problem_code.str.byte_len, problem_code.str.str);
	fprintf(stderr, "| %*s\n", 6+(int)info.colm+2-problem_code.tabs, "^");

	exit(1);
}

static void 
lexer_proc(Token_Stack *stack, utf8_string code, const char *fpath) 
{
	stack_init(stack, 10);
	
	int line      = 1;
	int character = 1;

	utf8_char_iter iter = make_utf8_char_iter(code);
	utf8_char ch;
	while ((ch = next_utf8_char(&iter)).byte_len > 0) {
	switch (unicode_code_point(ch)) {
		case '(': // (
			stack_push(stack, create_token(TOKEN_LEFT_BRACKET, line, character));
			break;
		case ')': // )
			stack_push(stack, create_token(TOKEN_RIGHT_BRACKET, line, character));
			break;
		case ';' : // ;
			stack_push(stack, create_token(TOKEN_SEMICOLON, line, character));
			break; 
		case ':': // :
			stack_push(stack, create_token(TOKEN_COLON, line, character));
			break;
		case ',' : // ,
			stack_push(stack, create_token(TOKEN_COMMA, line, character));
			break; 
		case '+': // +
			stack_push(stack, create_token(TOKEN_PLUS, line, character));
			break;
		case '-': // -
			stack_push(stack, create_token(TOKEN_MINUS, line, character));
			break;
		case '*': // *
			stack_push(stack, create_token(TOKEN_ASTERISK, line, character));
			break;
		case '/': // /
			stack_push(stack, create_token(TOKEN_SLASH, line, character));
			break;
		case '=': // = 
			stack_push(stack, create_token(TOKEN_ASSING, line, character));
			break;
		case '"': /*string*/ {
			utf8_char next = peek_utf8_char(&iter);	
			if (unicode_code_point(next) == '"') {
				stack_push(stack, create_token(TOKEN_STRING, line, character)); // EMPTY STR
				ch = next_utf8_char(&iter); // TAKE "
				character++;
				break;
			}

			owned_utf8_string str = owned_string_with_capacity(20);
			while (((next = peek_utf8_char(&iter)).byte_len > 0) && unicode_code_point(next) != '"') {
				ch = next_utf8_char(&iter);
				character++;
				owned_string_push_back(&str, ch);
			}
			stack_push(stack, (Code_Token) { .content = str, .type = TOKEN_STRING, .line = line, .colm = character });
			ch = next_utf8_char(&iter); // TAKE "
			character++;
			break;
		}
		case '\n': // new line
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
				while ((next = peek_utf8_char(&iter)).byte_len > 0 && (is_ident_char(next) || isdigit(unicode_code_point(next)))) {
					ch = next_utf8_char(&iter);
					owned_string_push_back(&str, ch);	
					character++;
				}
				int keyword_type = is_keyword(str.str);
				if (keyword_type < 0) {
					/* воспринимаем как идентификатор */
					stack_push(stack, (Code_Token) { .content = str, .type = TOKEN_IDENTIFIER, .line = line, .colm = character });
				} else {
					/* воспринимаем как ключ. слово*/
					stack_push(stack, (Code_Token) { .content = str, .type = (Token_Type)keyword_type, .line = line, .colm = character });
				}

				break;
			}
			if (isdigit(unicode_code_point(ch))) {
				owned_utf8_string str = owned_string_with_capacity(6);
				utf8_char next;

				owned_string_push_back(&str, ch);
				while ((next = peek_utf8_char(&iter)).byte_len > 0 && isdigit(unicode_code_point(next))) {
					ch = next_utf8_char(&iter);
					owned_string_push_back(&str, ch);	
					character++;
				}

				stack_push(stack, (Code_Token) { .content = str, .type = TOKEN_INT, .line = line, .colm = character });
				break;
			}
			error_code("Неизвестный символ", (Err_Info) { fpath, code, line, character });
			break;
	}	
	 	character += 1;
	}	

	stack_push(stack, create_token(TOKEN_EOF, line, character));
}

static int
is_file(const char *path)
{
	struct stat s; return (stat(path, &s) == 0) && (s.st_mode & S_IFREG);
}

static char
*read_file(const char *name) 
{
	if (!is_file(name)) return NULL;

	size_t size;
	FILE *f = fopen(name, "rb");
	if (!f) return NULL;	
	
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	char *buff = malloc(size+1);
	fread(buff, 1, size, f);
	fclose(f);
	buff[size] = '\0';
	return buff;
}

static void
print_help_and_exit() 
{
	printf("прк - Правда Компилятор.\n");
	printf("Компилятор для языка программирования 'правда'\n");
	printf("Использование: прк <файл>\n\n");
	printf("Это свободное программное обеспечение, вы можете смотреть, изменять код.\n");
	exit(1);
}

static void 
print_token(Code_Token *token) 
{
	printf("Токен{тип = '%s', содержание = '%s'};\n", Token_Names[token->type], token->content.str);
}

static void
ast_node_print(struct Ast_Node *node, int level)
{
	switch (node->kind) {
		case AST_BLOCK: {
			struct AST_BLOCK data = node->data.AST_BLOCK; 
			printf("%*sprogram\n", level*4, " ");
			for (size_t i = 0; i < data.size; ++i)
				ast_node_print(&data.block[i], level+1);
			break;
		}
		case AST_NUMBER: {
			struct AST_NUMBER data = node->data.AST_NUMBER; 
			printf("%*snumber(%i)\n", level*4, " ", data.value);
			break;
		}
		case AST_PLUS: {
			struct AST_PLUS data = node->data.AST_PLUS; 
			printf("%*splus: \n", level*4, " ");
			ast_node_print(data.lhs, level+1);
			ast_node_print(data.rhs, level+1);
			break;
		}
		case AST_MINUS: {
			struct AST_MINUS data = node->data.AST_MINUS; 
			printf("%*sminus: \n", level*4, " ");
			ast_node_print(data.lhs, level+1);
			ast_node_print(data.rhs, level+1);
			break;
		}
		case AST_MULTIPLE: {
			struct AST_MULTIPLE data = node->data.AST_MULTIPLE; 
			printf("%*smultiple: \n", level*4, " ");
			ast_node_print(data.lhs, level+1);
			ast_node_print(data.rhs, level+1);
			break;
		}
		case AST_DEVIDE: {
			struct AST_DEVIDE data = node->data.AST_DEVIDE; 
			printf("%*sdevide: \n", level*4, " ");
			ast_node_print(data.lhs, level+1);
			ast_node_print(data.rhs, level+1);
			break;
		} case AST_TYPE: {
			struct AST_TYPE data = node->data.AST_TYPE;
			printf("%*stype: %s\n", level*4, " ", Type_Names[data.type]);
			break;
		} case AST_VAR_DEF: {
			struct AST_VAR_DEF data = node->data.AST_VAR_DEF;
			printf("%*svariable: %s\n", level*4, " ", data.id.str);
			ast_node_print(data.type, level+1);
			ast_node_print(data.expr, level+1);
			break;
		}
		default:
			error_exit("Неизвестный тип в AST", 1);
			break;
	}
}

static struct Ast_Node *
ast_node_new(struct Ast_Node node)
{
	struct Ast_Node *ptr = malloc(sizeof(struct Ast_Node));
	if (ptr) *ptr = node;
	return ptr;
}

static void
ast_node_block(struct Ast_Node *block, size_t capacity)
{
	struct Ast_Node *tmp = malloc(sizeof(struct Ast_Node) * capacity);
	if (!tmp) return;

	block->kind = AST_BLOCK;	
	block->data.AST_BLOCK.block = tmp;
	block->data.AST_BLOCK.size  = 0;
	block->data.AST_BLOCK.capc  = capacity;
}

static void
ast_node_block_push_back(struct Ast_Node *block, struct Ast_Node *node)
{
	if (block->data.AST_BLOCK.capc <= block->data.AST_BLOCK.size) {
		struct Ast_Node *tmp = realloc(
			block->data.AST_BLOCK.block,
			block->data.AST_BLOCK.capc * sizeof(struct Ast_Node) * 2
		);
		if (!tmp) return;
		block->data.AST_BLOCK.block = tmp;
		block->data.AST_BLOCK.capc *= 2;
	}
	block->data.AST_BLOCK.size += 1;	
	block->data.AST_BLOCK.block[block->data.AST_BLOCK.size - 1] = *node;	
}


static struct Ast_Node
parser_proc(Token_Stack *tokens, const char *path, utf8_string code)
{
	struct Ast_Node root = {0};
	ast_node_block(&root, tokens->size);

	Token_Stack_Iter iter = stack_iter(tokens);
	
	while (stack_iter_has_next(&iter)) {
		switch (iter.current->type) {
			case TOKEN_KEYWORD_LET:
				ast_node_block_push_back(&root, parser_parse_let(&iter, path, code));
				stack_iter_next(&iter);
				stack_iter_accept(&iter, TOKEN_SEMICOLON, path, code);
				break;
			case TOKEN_INT:
				ast_node_block_push_back(&root, parser_parse_expr(0, &iter, path, code));
				stack_iter_next(&iter);
				stack_iter_accept(&iter, TOKEN_SEMICOLON, path, code);
				break;
			default:
				error_code("Not implemented.", (Err_Info){ path, code, iter.current->line, iter.current->colm});
				break;
		}
	}
	return root;
}

static int 
is_operator(Token_Type type)
{
	return type == TOKEN_PLUS || type == TOKEN_MINUS 
		|| type == TOKEN_ASTERISK || type == TOKEN_SLASH;
}

static struct Ast_Node *new_node_from_operator(
	struct Ast_Node *lhs, struct  Ast_Node *rhs, Token_Type op)
{
	switch (op) {
		case TOKEN_PLUS:
			return AST_NODE_NEW(AST_PLUS, lhs, rhs);
		case TOKEN_MINUS:
			return AST_NODE_NEW(AST_MINUS, lhs, rhs);
		case TOKEN_ASTERISK:
			return AST_NODE_NEW(AST_MULTIPLE, lhs, rhs);
		case TOKEN_SLASH:
			return AST_NODE_NEW(AST_DEVIDE, lhs, rhs);
		default:
			error_exit("new_node_from_operator: Не известный тип токена!", 1);
			return NULL; // никогда не произойдет
			break;
	}
}

static int 
l_binding_power(Token_Type type)
{
	int res = 0;
	switch (type) {
		case TOKEN_PLUS: /* FALL THROUGH */
		case TOKEN_MINUS:
			res = 1;
			break;
		case TOKEN_ASTERISK: /* FALL THROUGH */
		case TOKEN_SLASH:
			res = 3;
			break;
		default:
			fprintf(stderr, "Оператор не распознан: %s\n", Token_Names[type]);
			break;
	}
	return res;
}

static int
r_binding_power(Token_Type type)
{
	int res = 0;
	switch (type) {
		case TOKEN_PLUS: /* FALL THROUGH */
		case TOKEN_MINUS:
			res = 2;
			break;
		case TOKEN_ASTERISK: /* FALL THROUGH */
		case TOKEN_SLASH:
			res = 4;
			break;
		default:
			fprintf(stderr, "Оператор не распознан: %s\n", Token_Names[type]);
			break;
	}
	return res;
}

/* Pratt parsing */
static struct Ast_Node *
parser_parse_expr(int min_bp, Token_Stack_Iter *iter, const char *path, utf8_string code)
{	
	if (iter->current->type != TOKEN_IDENTIFIER && iter->current->type != TOKEN_INT)
		error_code("Ожидалась константа или идентификатор.", (Err_Info){path, code, iter->current->line, iter->current->colm});

	struct Ast_Node *lhs = AST_NODE_NEW(AST_NUMBER, atoi(iter->current->content.str));
		
	while (1) {
		Token_Type op = stack_iter_peek(iter)->type;
		if (op == TOKEN_EOF || op == TOKEN_SEMICOLON) break;
		if (!is_operator(op))
			error_code("Ожидается оператор или ';' после выржания.", (Err_Info){path, code, iter->current->line, iter->current->colm});

	//      1                          2	
		int lbp = l_binding_power(op), rbp = r_binding_power(op);
		if (lbp < min_bp) {
			break;
		}	
		stack_iter_next(iter);// operator
		stack_iter_next(iter);// const

		struct Ast_Node *rhs = parser_parse_expr(rbp, iter, path, code);
		
		lhs = new_node_from_operator(lhs, rhs, op);
	}
	
	return lhs;
}

static struct Ast_Node *
parser_parse_type(Token_Stack_Iter *iter, const char *path, utf8_string code)
{
	stack_iter_check(iter, TOKEN_IDENTIFIER, path, code);
	owned_utf8_string type = iter->current->content;

	if (strcmp(type.str, "цел") == 0 || strcmp(type.str, "цел32") == 0) {
		return AST_NODE_NEW(AST_TYPE, TYPE_INT_32);
	} else if (strcmp(type.str, "цел8") == 0) {
		return AST_NODE_NEW(AST_TYPE, TYPE_INT_8);
	} else if (strcmp(type.str, "цел16") == 0) {
		return AST_NODE_NEW(AST_TYPE, TYPE_INT_16);
	} else if (strcmp(type.str, "нат") == 0 || strcmp(type.str, "нат32") == 0) {
		return AST_NODE_NEW(AST_TYPE, TYPE_UINT_32);
	} else if (strcmp(type.str, "нат16") == 0) {
		return AST_NODE_NEW(AST_TYPE, TYPE_UINT_16);
	} else if (strcmp(type.str, "нат8") == 0) {
		return AST_NODE_NEW(AST_TYPE, TYPE_UINT_8);
	} else {
		error_code("Неизвестный тип переменной!", (Err_Info){path, code, iter->current->line, iter->current->colm});
		return NULL;
	}
}

static struct Ast_Node *
parser_parse_let(Token_Stack_Iter *iter, const char *path, utf8_string code) 
{
	stack_iter_accept(iter, TOKEN_KEYWORD_LET, path, code);

	stack_iter_check(iter, TOKEN_IDENTIFIER, path, code);
	owned_utf8_string id = iter->current->content;
	stack_iter_next(iter);

	stack_iter_accept(iter, TOKEN_COLON, path, code);
	struct Ast_Node *type = parser_parse_type(iter, path, code);
	stack_iter_next(iter);
	
	stack_iter_accept(iter, TOKEN_ASSING, path, code);
	struct Ast_Node *expr = parser_parse_expr(0, iter, path, code);

	return AST_NODE_NEW(AST_VAR_DEF, id, type, expr);
}

int
main(int argc, char **argv)
{
	if (argc < 2) print_help_and_exit();
	owned_utf8_string code = make_utf8_string_lossy(read_file(argv[1]));
	if (code.str == NULL) error_exit("Неудалось открыть файл, возможно, его не существует или это директория.", 1);
	
	Token_Stack stack = {0};
	lexer_proc(&stack, as_utf8_string(&code), argv[1]);	

	printf("Код:\n%s\n", code.str);

	for (size_t i = 0; i < stack.size; ++i) {
		print_token(&stack.toks[i]);
	}
	printf("\n");

	struct Ast_Node ast = parser_proc(&stack, argv[1], as_utf8_string(&code));
	ast_node_print(&ast, 0);

	free_owned_utf8_string(&code);
	return EXIT_SUCCESS;
}
