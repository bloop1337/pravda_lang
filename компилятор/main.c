#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lexer.h"


// выделяет память через malloc
// возвращает NULL в случаее не удачи
static char *pravda_read_file(const char *name);

static void pravda_print_help_and_exit();
static void pravda_print_error_and_exit(const char *what);

// цель: написать минимальный компилятор,
// чтоб начать писать компилятор на правде
int main(int argc, char** argv) 
{
	if (argc < 2) pravda_print_help_and_exit();
 	char *code = pravda_read_file(argv[1]);
	if (code == NULL) pravda_print_error_and_exit("Файл не найден.");

	Pravda_Token *tokens = NULL;
	size_t tokens_length = 0;
	pravda_lexer_process(tokens, &tokens_length, code);
	
	for (size_t i = 0; i < tokens_length; ++i)
		printf("%s", tokens[i].content);

	return 0;
}

static char *pravda_read_file(const char *name) {
	// read
	return NULL;
}

static void pravda_print_help_and_exit() {
	printf("прк - Правда Компилятор.\n");
	printf("Компилятор для языка программирования 'правда'\n");
	printf("Использование: прк <файл>\n\n");
	printf("Это свободное программное обеспечение, вы можете смотреть, изменять код.\n");
	exit(1);
}

static void pravda_print_error_and_exit(const char *what) {
	printf("Ошибка при исполнении команды: %s\n", what);
	exit(1);
}

