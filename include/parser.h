#ifndef PARSER_H
#define PARSER_H

#include "lizard.h"

void forward();
void consume(TypeToken type, char *message);
double expression();
double arith_expr();
double term();
double factor();
void assignation();
void print_stmt();
void if_stmt();
void block();
void skip_block();
void program();

#endif