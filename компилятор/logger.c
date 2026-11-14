#include "./thirdparty/utf8.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>

#define PRAVDA_NUMBER_WIDTH 6
#define PRAVDA_RESET 0
#define PRAVDA_BOLD 1
#define PRAVDA_ERROR_COLOR 31
#define PRAVDA_WARN_COLOR 33

static utf8_string
get_utf8_line(utf8_string code, int line) {
    utf8_char_iter iter = make_utf8_char_iter(code);
    utf8_char ch;
    int current_line = 1;
    size_t start_index = 0;

    while (current_line < line) {
        ch = next_utf8_char(&iter);
        if (ch.byte_len == 0) return make_utf8_string("");
        if (unicode_code_point(ch) == 0x000A) current_line++;
        start_index += ch.byte_len;
    }

    size_t end_index = start_index;
    while ((ch = next_utf8_char(&iter)).byte_len > 0) {
        if (unicode_code_point(ch) == 0x000A) break;
		if (unicode_code_point(ch) == 0x0009) start_index++;
        end_index += ch.byte_len;
    }
    return slice_utf8_string(code, start_index, end_index - start_index);
}

void
pravda_warn(const char *what) 
{
	fprintf(stderr, "\x1B[%i;%imпредупреждение:\x1B[%i;%im %s\x1B[%im\n", PRAVDA_BOLD, PRAVDA_WARN_COLOR, PRAVDA_RESET, PRAVDA_BOLD, what, PRAVDA_RESET);
}

void
pravda_error(const char *what)
{
	fprintf(stderr, "\x1B[%i;%imошибка: \x1B[%i;%im%s\x1B[%im\n", PRAVDA_BOLD, PRAVDA_ERROR_COLOR, PRAVDA_RESET, PRAVDA_BOLD, what, PRAVDA_RESET);
	exit(1);
}

void
pravda_warn_code(const char *fpath, utf8_string code, const char *what, int line, int character)
{
	fprintf(stderr, "%s:%i:%i предупреждение: %s\n|\n", fpath, line, character, what);
	utf8_string problem_code = get_utf8_line(code, line);

	fprintf(stderr, "| %*i: %.*s\n", PRAVDA_NUMBER_WIDTH, line, (int)problem_code.byte_len, problem_code.str);
	fprintf(stderr, "| %*s\n", PRAVDA_NUMBER_WIDTH+character+2, "^");
}

void
pravda_error_code(const char *fpath, utf8_string code, const char *what, int line, int character)
{
	fprintf(stderr, "\x1B[%im%s:%i:%i \x1B[%imошибка:\x1B[%im \x1B[%im%s\x1B[%im\n|\n", PRAVDA_BOLD, fpath, line, character, PRAVDA_ERROR_COLOR, PRAVDA_RESET, PRAVDA_BOLD, what, PRAVDA_RESET);

	utf8_string problem_code = get_utf8_line(code, line);

	fprintf(stderr, "| %*i: %.*s\n", PRAVDA_NUMBER_WIDTH, line, (int)problem_code.byte_len, problem_code.str);
	fprintf(stderr, "| %*s\n", PRAVDA_NUMBER_WIDTH+character+2, "^");

	exit(1);
}
