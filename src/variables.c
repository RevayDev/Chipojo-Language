#include "variables.h"

#define MAX_VARS 100

Scope scope_stack[MAX_SCOPE];
int scope_depth = 0;


 void assignNumberVar(char *name, double num)
{
    Scope *sc = &scope_stack[scope_depth];
    for (int i = 0; i < sc->num_vars; i++)
        if (strcmp(sc->vars[i].name, name) == 0)
        {
            sc->vars[i].type = VAR_NUMBER;
            sc->vars[i].value.num = num;

            return;
        }
    strcpy(sc->vars[sc->num_vars].name, name);
    sc->vars[sc->num_vars].type = VAR_NUMBER;
    sc->vars[sc->num_vars].value.num = num;
    sc->num_vars++;
}

void assignStringVar(char *name, char *num)
{
    Scope *sc = &scope_stack[scope_depth];

    for (int i = 0; i < sc->num_vars; i++)
        if (strcmp(sc->vars[i].name, name) == 0)
        {
            sc->vars[i].type = VAR_STRING;
            strcpy(sc->vars[i].value.str, num);
            return;
        }
    strcpy(sc->vars[sc->num_vars].name, name);
    sc->vars[sc->num_vars].type = VAR_STRING;
    strcpy(sc->vars[sc->num_vars].value.str, num);
    sc->num_vars++;
}

void function_definition(char *name, int start,char **params, int param_count)
{
    Scope *sc = &scope_stack[scope_depth];
    for (int i = 0; i < sc->num_vars; i++)
        if (strcmp(sc->vars[i].name, name) == 0)
        {
            char error[256];
            snprintf(error, sizeof(error), "Function %s is defined twice", name);
            syntax_error_line(error, current_token.line);
            exit(1);
        }
    
    strcpy(sc->vars[sc->num_vars].name, name);
    sc->vars[sc->num_vars].type = VAR_FUNCTION;
    sc->vars[sc->num_vars].func.start = start;

    sc->vars[sc->num_vars].func.param = malloc(param_count * sizeof(char *));
    for (int i = 0; i < param_count; i++)
        sc->vars[sc->num_vars].func.param[i] = strdup(params[i]);
    
    sc->vars[sc->num_vars].func.param_count = param_count;
    sc->num_vars++;
}

void assignNullVar(char *name)
{
    Scope *sc = &scope_stack[scope_depth];
    for (int i = 0; i < sc->num_vars; i++)

        if (strcmp(sc->vars[i].name, name) == 0)
        {
            sc->vars[i].type = VAR_NULL;
            return;
        }
    strcpy(sc->vars[sc->num_vars].name, name);
    sc->vars[sc->num_vars].type = VAR_NULL;
    sc->num_vars++;
    }

Value getVarValue(char *name)
{
    Value v;
    for (int d = scope_depth; d >= 0; d--)
    {
        Scope *sc = &scope_stack[d];
        for (int i = 0; i < sc->num_vars; i++)
            if (strcmp(sc->vars[i].name, name) == 0)
            {
                strcpy(v.name, sc->vars[i].name);

                v.type = sc->vars[i].type;
                if (v.type == VAR_NUMBER)
                    v.value.num = sc->vars[i].value.num;
                if (v.type == VAR_STRING)
                    strcpy(v.value.str, sc->vars[i].value.str);
                if (v.type == VAR_FUNCTION){
                    v.func.start = sc->vars[i].func.start;
                    v.func.param = sc->vars[i].func.param;
                    v.func.param_count = sc->vars[i].func.param_count;
                }
                return v;
            }
    }
    undefined_variable_error(name, current_token.line);
    return v;
}

void setVariable(char *name, Value value){

    if (value.type == VAR_STRING)
    {
        assignStringVar(name, value.value.str);
        }
    else if (value.type == VAR_NULL)
    {
        assignNullVar(name);
    }
    else if (value.type == VAR_NUMBER)
    {
        assignNumberVar(name, value.value.num);
    }
    else{
        syntax_error("Unexpected value",current_token);
    }
}

void pushScope(){
    scope_depth++;

    scope_stack[scope_depth].num_vars = 0;
}
void popScope(){
    if (scope_depth > 0) scope_depth--;
}