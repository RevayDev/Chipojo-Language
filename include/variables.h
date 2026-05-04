#ifndef VARIABLES_H
#define VARIABLES_H

#include "lizard.h"
#include "error.h"

void assignIntVar(char *name, int val);
void assignStringVar(char *name, char *val);
int getIntVar(char *name);
char *getStringVar(char *name);
#endif