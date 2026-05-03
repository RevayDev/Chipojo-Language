#ifndef LEXER_H
#define LEXER_H

#include "lizard.h"

void nextChar();
char currentChar();
void jumpBlankspace();
void jumpBOM();
TypeToken peek_next_token_type(void);
Token nextToken();
#endif