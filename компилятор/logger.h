#ifndef PRAVDA_LOGGER
#define PRAVDA_LOGGER 1

void pravda_warn_code(const char *fpath, utf8_string code, const char *what, int line, int character);
void pravda_error_code(const char *fpath, utf8_string code, const char *what, int line, int character);

void pravda_warn(const char *what);
void pravda_error(const char *what);
#endif
