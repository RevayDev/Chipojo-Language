#ifndef PARSER_H
#define PARSER_H

#include "lizard.h"
#include "variables.h"

void forward();
void consume(TypeToken type, char *message);
Value expression();
Value arith_expr();
Value term();
Value factor();
void assignation();
void print_stmt();
void if_stmt();
void block();
void skip_block();
void program();

#endif