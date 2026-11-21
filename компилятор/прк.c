#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/stat.h>

#define UTF8_IMPL
#include "ютф8.h"

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
	utf8_string str;
	unsigned tabs;
} Code_Line;

typedef struct {
	const char *path;
	utf8_string code;

	size_t line;
	size_t colm;
} Err_Info;

/*functions decl*/
static char *read_file(const char *name);
static int is_file(const char *path);

static Code_Token create_token(Token_Type type, int line, int colm);

static void stack_init(Token_Stack *stack, size_t capacity);
static void stack_push(Token_Stack *stack, Code_Token token);
static void stack_free(Token_Stack *stack);

static unsigned int is_keyword(const char* s);
static int  is_ident_char(utf8_char ch);
static void lexer_proc(Token_Stack *tokens, utf8_string code, const char* fpath);

static void error_code(const char *what, Err_Info info);
static void print_token(Code_Token *token);

/*constants*/
static char *keywords[] = { "фн", "верни", "если", "пока", "конец", "пусть", };

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

int
main(int argc, char **argv)
{
	if (argc < 2) print_help_and_exit();
	owned_utf8_string code = make_utf8_string_lossy(read_file(argv[1]));
	if (code.str == NULL) error_code("Неудалось открыть файл!", (Err_Info){0});
	
	Token_Stack stack = {0};
	lexer_proc(&stack, as_utf8_string(&code), argv[1]);	

	for (size_t i = 0; i < stack.size; ++i) {
		print_token(&stack.toks[i]);
	}

	free_owned_utf8_string(&code);
	return EXIT_SUCCESS;
}
