#include "variables.h"

#define MAX_VARS 100
Var vars_table[MAX_VARS];
int num_vars = 0;

void assignNumberVar(char *name, double val)
{
    for (int i = 0; i < num_vars; i++)
        if (strcmp(vars_table[i].name, name) == 0)
        {
            vars_table[i].type = VAR_NUMBER;
            vars_table[i].value.val = val;
            return;
        }
    strcpy(vars_table[num_vars].name, name);
    vars_table[num_vars].type = VAR_NUMBER;
    vars_table[num_vars].value.val = val;
    num_vars++;
}

void assignStringVar(char *name, char *val)
{
    for (int i = 0; i < num_vars; i++)
        if (strcmp(vars_table[i].name, name) == 0)
        {
            vars_table[i].type = VAR_STRING;
            strcpy(vars_table[i].value.str_val, val);
            return;
        }
    strcpy(vars_table[num_vars].name, name);
    vars_table[num_vars].type = VAR_STRING;
    strcpy(vars_table[num_vars].value.str_val, val);
    num_vars++;
}

double getNumberVar(char *name)
{
    for (int i = 0; i < num_vars; i++)
        if (strcmp(vars_table[i].name, name) == 0)
        {
            if (vars_table[i].type != VAR_NUMBER)
            {
                printf("Error: variable '%s'  no integer \n", name);
                exit(1);
            }
            return vars_table[i].value.val;
        }
    undefined_variable_error(name,current_token.line);
    exit(1);
}

char *getStringVar(char *name)
{
    for (int i = 0; i < num_vars; i++)
        if (strcmp(vars_table[i].name, name) == 0)
        {
            if (vars_table[i].type != VAR_STRING)
            {
                printf("Error: variable '%s' no string\n", name);
                exit(1);
            }
            return vars_table[i].value.str_val;
        }
    undefined_variable_error(name,current_token.line);
    exit(1);
}
