#include "parser.h"
#include "lexer.h"
#include "variables.h"
#include "error.h"

Token current_token;

void forward() { current_token = nextToken(); }
void consume(TypeToken type, char *message)
{
    if (current_token.type == type)
        forward();
    else
    {
        syntax_error("",current_token);   
        exit(1);
    }
}

// Prototypes
double comparison_op(double left, TypeToken op, double right);
void concat_element(char *buffer, size_t buffsize);
void print_concat();

void if_stmt()
{
    consume(TOKEN_IF, "if error");
    double cond = expression();
    consume(TOKEN_LEFTBRACE, "{ error");

    int executed = 0;

    if (cond != 0)
    {
        // Execute if block
        block();
        executed = 1;
    }
    else
    {
        skip_block();
    }

    while (current_token.type == TOKEN_ELIF)
    {
        forward();
        double elif_cond = expression();
        consume(TOKEN_LEFTBRACE, "{ error");
        if (!executed && elif_cond != 0)
        {
            block();
            executed = 1;
        }
        else
        {
            skip_block();
        }
    }

    // if exist else
    if (current_token.type == TOKEN_ELSE)
    {
        forward();
        consume(TOKEN_LEFTBRACE, "{ error");
        if (executed == 0)
        {
            // Execute if block
            block();
        }
        else
        {
            skip_block();
        }
    }
}

void while_stmt(void)
{
    int after_cond = indx;
    int after_while = -1;
    while (1)
    {
        indx = after_cond;
        forward();
        double cond = expression();
        consume(TOKEN_LEFTBRACE, "Error {");
        if (cond != 0)
        {
            block();
        }
        else
        {
            skip_block();
            after_while = indx;
            break;
        }
    }
    indx = after_while;
}

void skip_block()
{
    int brace_count = 1;
    while (brace_count > 0)
    {
        if (current_token.type == TOKEN_LEFTBRACE)
            brace_count++;
        else if (current_token.type == TOKEN_RIGHTBRACE)
            brace_count--;
        if (brace_count == 0)
            break;
        forward();
    }
    forward(); // consume
}

void block()
{
    while (current_token.type != TOKEN_RIGHTBRACE && current_token.type != TOKEN_EOF)
    {
        if (current_token.type == TOKEN_PRINT)
            print_stmt();
        else if (current_token.type == TOKEN_ID)
            assignation();
        else if (current_token.type == TOKEN_IF)
            if_stmt();
        else if (current_token.type == TOKEN_WHILE)
            while_stmt();
        else
        {
            printf("Error invalid sentence in block (token %d)\n", current_token.type);
            exit(1);
        }
    }
    consume(TOKEN_RIGHTBRACE, "error }");
}

double expression()
{
    double left = arith_expr();
    while (current_token.type == TOKEN_EQ || current_token.type == TOKEN_NE ||
           current_token.type == TOKEN_LT || current_token.type == TOKEN_GT ||
           current_token.type == TOKEN_LE || current_token.type == TOKEN_GE)
    {
        TypeToken op = current_token.type;
        forward();
        double right = arith_expr();
        left = comparison_op(left, op, right);
    }
    return left;
}

double arith_expr()
{
    double left = term();
    while (current_token.type == TOKEN_SUM || current_token.type == TOKEN_REST)
    {
        if (current_token.type == TOKEN_SUM)
        {
            forward();
            left += term();
        }
        else
        {
            forward();
            left -= term();
        }
    }
    return left;
}

double comparison_op(double left, TypeToken op, double right)
{
    switch (op)
    {
    case TOKEN_EQ:
        return left == right;
    case TOKEN_NE:
        return left != right;
    case TOKEN_LT:
        return left < right;
    case TOKEN_GT:
        return left > right;
    case TOKEN_LE:
        return left <= right;
    case TOKEN_GE:
        return left >= right;
    default:
        return 0;
    }
}

double term()
{
    double left = factor();
    while (current_token.type == TOKEN_MUL || current_token.type == TOKEN_DIV)
    {
        Token token = current_token;

        if (current_token.type == TOKEN_MUL)
        {
            forward();
            left *= factor();
        }
        else
        {
            forward();
            double divisor = factor();
            if (divisor == 0)
            {
                syntax_error("Division by zero", token);
                exit(1);
            }
            left /= divisor;
        }
    }
    return left;
}

double factor()
{ // Operator Prefix ++ --
    if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
    {
        TypeToken op = current_token.type;
        forward();
        if (current_token.type != TOKEN_ID)
        {
            syntax_error("Syntax Error", current_token);
            exit(1);
        }

        char name[64];
        strcpy(name, current_token.name);
        double old = getNumberVar(name);
        double new_val = (op == TOKEN_INC) ? old + 1 : old - 1;
        assignNumberVar(name, new_val);
        forward();
        return new_val;
    }
    // Boolean
    if (current_token.type == TOKEN_TRUE)
    {
        forward();
        return 1;
    }
    if (current_token.type == TOKEN_FALSE)
    {
        forward();
        return 0; 
    }
    // Number
    else if (current_token.type == TOKEN_NUM)
    {
        double val = current_token.value;
        forward();
        return val;
    }
    else if (current_token.type == TOKEN_ID)
    {
        char name[64];
        strcpy(name, current_token.name);
        forward();
        if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
        {
            TypeToken op = current_token.type;
            forward();
            double old = getNumberVar(name);
            double new_val = (op == TOKEN_INC) ? old + 1 : old - 1;
            assignNumberVar(name, new_val);
            return old;
        }

        return getNumberVar(name);
    }
    else if (current_token.type == TOKEN_PARENTLEFT)
    {
        forward();
        double val = expression();
        consume(TOKEN_PARENTRIGHT, "Falta ')'");
        return val;
    }
    else
    {
        syntax_error("Syntax Error", current_token);
        exit(1);
    }
}

void assign_compound(char *name, TypeToken op, double val, int op_line)
{
    double current = getNumberVar(name);
    double result;
    switch (op)
    {
    case TOKEN_PLUS_ASSIGN:
        result = current + val;
        break;
    case TOKEN_MINUS_ASSIGN:
        result = current - val;
        break;
    case TOKEN_MULT_ASSIGN:
        result = current * val;
        break;
    case TOKEN_DIV_ASSIGN:
        if (val == 0)
        {
            printf("Error in line %d:Division by zero",op_line);
                exit(1);
        }
        result = current / val;
        break;

    default:
        return;
    }
    assignNumberVar(name, result);
}

void assignation()
{
    char name[64];
    strcpy(name, current_token.name);

    if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
    {
        TypeToken op = current_token.type;
        forward();
        if (current_token.type != TOKEN_ID)
        {
            printf("Error: se esperaba identificador\n");
            exit(1);
        }
        strcpy(name, current_token.name);
        forward();
        double old = getNumberVar(name);
        double new_val = (op == TOKEN_INC) ? old + 1 : old - 1;
        assignNumberVar(name, new_val);
        return;
    }

    if (current_token.type == TOKEN_ID)
    {
        strcpy(name, current_token.name);
        forward(); //Consume ID
        if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
        {
            TypeToken op = current_token.type;
            forward();
            double old = getNumberVar(name);
            double new_val = (op == TOKEN_INC) ? old + 1 : old - 1;
            assignNumberVar(name, new_val);
            return;
        }
    }

    if (current_token.type == TOKEN_PLUS_ASSIGN ||
        current_token.type == TOKEN_MINUS_ASSIGN ||
        current_token.type == TOKEN_DIV_ASSIGN ||
        current_token.type == TOKEN_MULT_ASSIGN)
    {
        TypeToken op = current_token.type;
        double current_line = current_token.line;
        forward();
        double val = expression();
        assign_compound(name, op, val,current_line);
        return;
    }

    consume(TOKEN_ASIGN, "not found '='");

    if (current_token.type == TOKEN_STRING)
    {
        assignStringVar(name, current_token.name);
        forward();
    }
    else
    {
        double val = expression();
        assignNumberVar(name, val);
    }
}

void concat_element(char *buffer, size_t buffsize)
{
    char temp[64];

    if (current_token.type == TOKEN_STRING)
    {
        strncat(buffer, current_token.name, buffsize - strlen(buffer) - 1);
        forward();
    }
    else if (current_token.type == TOKEN_ID)
    {
        char var_name[64];
        strcpy(var_name, current_token.name);
        TypeToken next = peek_next_token_type();
        if (next == TOKEN_INC || next == TOKEN_DEC)
        {
            double val = factor();
            snprintf(temp, sizeof(temp), "%g", val);
            strncat(buffer, temp, buffsize - strlen(buffer) - 1);
        }
        else
        {
            forward();
            int found = 0;
            for (int i = 0; i < num_vars; i++)
            {
                if (strcmp(vars_table[i].name, var_name) == 0)
                {
                    if (vars_table[i].type == VAR_STRING)
                    {
                        strncat(buffer, vars_table[i].value.str_val, buffsize - strlen(buffer) - 1);
                    }
                    else
                    {
                        snprintf(temp, sizeof(temp), "%g", vars_table[i].value.val);
                        strncat(buffer, temp, buffsize - strlen(buffer) - 1);
                    }
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                undefined_variable_error(var_name,current_token.line);
                exit(1);
            }
        }
    }
    else
    {
        double val = factor();
        snprintf(temp, sizeof(temp), "%g", val);
        strncat(buffer, temp, buffsize - strlen(buffer) - 1);
    }
}

void print_concat()
{
    char buffer[4096] = "";
    concat_element(buffer, sizeof(buffer));

    while (current_token.type == TOKEN_SUM)
    {
        forward();
        concat_element(buffer, sizeof(buffer));
    }

    printf("%s\n", buffer);
}

void print_stmt()
{
    consume(TOKEN_PRINT, "not found 'print'");
    consume(TOKEN_PARENTLEFT, "not found '('");
    print_concat();
    consume(TOKEN_PARENTRIGHT, "no found')'");
}

void program()
{
    while (current_token.type != TOKEN_EOF)
    {
        if (current_token.type == TOKEN_PRINT)
        {
            print_stmt();
        }
        else if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
        {
            expression();
        }
        else if (current_token.type == TOKEN_ID)
        {
            assignation();
        }
        else if (current_token.type == TOKEN_IF)
        {
            if_stmt();
        }
        else if (current_token.type == TOKEN_WHILE)
        {
            while_stmt();
        }
        else
        {
            printf("Error, invalid statement (token %d)\n", current_token.type);
            exit(1);
        }
    }
}