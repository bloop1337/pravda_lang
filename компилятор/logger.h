#ifndef PRAVDA_LOGGER
#define PRAVDA_LOGGER 1

void pravda_warn(const char *fpath, utf8_string code, const char *what, int line, int character);
void pravda_error(const char *fpath, utf8_string code, const char *what, int line, int character);

#endif
