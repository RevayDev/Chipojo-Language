#ifndef ERROR_H
#define ERROR_H

#include "lizard.h"

const char *expected_string(TypeToken type);
void syntax_error(const char *message, Token token);
void undefined_variable_error(const char *var_name, int line);
void type_error(const char *var_name, const char *expected, const char *found, int line);

#endif