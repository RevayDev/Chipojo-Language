#ifndef VARIABLES_H
#define VARIABLES_H

#include "lizard.h"
#include "error.h"
#define MAX_SCOPE 1000

// Variables
typedef enum
{
    VAR_NUMBER,
    VAR_STRING,
    VAR_FUNCTION,
    VAR_NULL
} VarType;


typedef struct
{
    char name[64];
    VarType type;
    union
    {
        double num;
        char str[256];
    } value;
    
    struct 
    {
        int start ;
        char ** param;
        int param_count;
    } func;

} Value;

typedef struct
{
    Value vars[MAX_SCOPE];
    int num_vars;

} Scope;

void assignNumberVar(char *name, double val);
void assignStringVar(char *name, char *val);
void function_definition(char *name, int start,char** params,int param_count);
void assignNullVar(char *name);
Value getVarValue(char *name);
Value getFunction(char *name);
void setVariable(char *name, Value value);
void pushScope();
void popScope();

extern Scope scope_stack[MAX_SCOPE];
extern int scope_depth;

#endif