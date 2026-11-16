#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lexer.h"
#include "logger.h"


static const char *Pravda_Token_Names[] = {
	[PRAVDA_KEYWORD]             = "PRAVDA_KEYWORD",
	[PRAVDA_IDENTIFIER]          = "PRAVDA_IDENTIFIER",
	[PRAVDA_INT]                 = "PRAVDA_INT",
	[PRAVDA_FLOAT]               = "PRAVDA_FLOAT",
	[PRAVDA_LEFT_BRACKET]        = "PRAVDA_LEFT_BRACKET",
	[PRAVDA_RIGHT_BRACKET]       = "PRAVDA_RIGHT_BRACKET",
	[PRAVDA_LEFT_CURLY_BRACKET]  = "PRAVDA_LEFT_CURLY_BRACKET",
	[PRAVDA_RIGHT_CURLY_BRACKET] = "PRAVDA_RIGHT_CURLY_BRACKET",
	[PRAVDA_PLUS]                = "PRAVDA_PLUS",
	[PRAVDA_MINUS]               = "PRAVDA_MINUS",
	[PRAVDA_SLASH]               = "PRAVDA_SLASH",
	[PRAVDA_ASTERISK]            = "PRAVDA_ASTERISK",
	[PRAVDA_COMMA]               = "PRAVDA_COMMA",
	[PRAVDA_SEMICOLON]           = "PRAVDA_SEMICOLON",
	[PRAVDA_POINT]               = "PRAVDA_POINT",
	[PRAVDA_STRING]              = "PRAVDA_STRING",
	[PRAVDA_ASSING]              = "PRAVDA_ASSING",
	[PRAVDA_EQUAL]               = "PRAVDA_EQUAL",
	[PRAVDA_GREATER_THAN]        = "PRAVDA_GREATER_THAN",
	[PRAVDA_LESS_THAN]           = "PRAVDA_LESS_THAN",
	[PRAVDA_LESS_OR_EQ_THAN]     = "PRAVDA_LESS_OR_EQ_THAN",
	[PRAVDA_GREATER_OR_EQ_THAN]  = "PRAVDA_GREATER_OR_EQ_THAN",
	[PRAVDA_COLON]  			 = "PRAVDA_COLON",
};
// выделяет память через malloc
// возвращает NULL в случаее не удачи
static char *pravda_read_file(const char *name);

static void pravda_print_help_and_exit();
static void pravda_print_token(Pravda_Token *token);

// цель: написать минимальный компилятор,
int
main(int argc, char** argv) 
{
	if (argc < 2) pravda_print_help_and_exit();
 	char *code = pravda_read_file(argv[1]);
	if (code == NULL) pravda_error("Файл не найден.");

	utf8_string ucode = make_utf8_string(code);
	Pravda_Token_Stack stack = {0};
	pravda_lexer_process(&stack, ucode, argv[1]);

	printf("Lexer result for: \n'\n%s'\n\n", ucode.str);
	for (size_t i = 0; i < stack.size; ++i) {
		pravda_print_token(&stack.tokens[i]);
	}
	free(code);	
	return 0;
}

bool is_file(const char *path) {
	struct stat s; return (stat(path, &s) == 0) && (s.st_mode & S_IFREG);
}

static char
*pravda_read_file(const char *name) 
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
pravda_print_help_and_exit() 
{
	printf("прк - Правда Компилятор.\n");
	printf("Компилятор для языка программирования 'правда'\n");
	printf("Использование: прк <файл>\n\n");
	printf("Это свободное программное обеспечение, вы можете смотреть, изменять код.\n");
	exit(1);
}

static void 
pravda_print_token(Pravda_Token *token) 
{
	printf("TOKEN{type = '%s', content = '%s'};\n", Pravda_Token_Names[token->type], token->content.str);
}
