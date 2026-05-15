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
    Value logical_or(void);
    Value logical_and(void);
    Value comparison_op(Value left, TypeToken op, Value right,int line);
    void concat_element(char *buffer, size_t buffsize);
    void print_concat();

    int is_truthy(Value v) {
        if (v.type == VAR_NUMBER) return v.value.num != 0;
        if (v.type == VAR_STRING) return v.value.str[0] != '\0';
        return 0; // Null values
    }

    Value expression(){
        return logical_or();
    }

    Value logical_or(void) {
        Value left = logical_and();
        while (current_token.type == TOKEN_OR) {
            forward();
            Value right = logical_and();
            if (is_truthy(left)) {
                Value v; v.type = VAR_NUMBER; v.value.num = 1; return v;
            } else {
                int truthy = is_truthy(right);
                Value v; v.type = VAR_NUMBER; v.value.num = truthy ? 1 : 0;
                return v;
            }
        }
        return left;
    }

    Value logical_and(void) {
        Value left = comparison_expr(); 
        while (current_token.type == TOKEN_AND) {
            forward();
            Value right = comparison_expr();
            if (!is_truthy(left)) {
                
                Value v; v.type = VAR_NUMBER; v.value.num = 0; return v;
            } else {
                int truthy = is_truthy(right);
                Value v; v.type = VAR_NUMBER; v.value.num = truthy ? 1 : 0;
                return v;
            }
        }
        return left;
    }

    void defineNewFunction()
    {

        consume(TOKEN_FUNC, "expected func");
        Token t = current_token;
        char name[64];
        consume(TOKEN_ID, "expected indentificator ");
        strcpy(name, t.name);
        char *params[100];
        int param_count = 0;
        consume(TOKEN_PARENTLEFT, "expected (");
        while (current_token.type != TOKEN_PARENTRIGHT)
        {
            if (current_token.type == TOKEN_ID)
            {
                params[param_count] = strdup(current_token.name);
                param_count++;
            }
            forward();
            }
            consume(TOKEN_PARENTRIGHT, "expected indentificator )");
            int start = indx;
            consume(TOKEN_LEFTBRACE, "expected indentificator {");

            int braces = 1;
            while (braces > 0)
            {
                if (current_token.type == TOKEN_LEFTBRACE)
                    braces++;
                else if (current_token.type == TOKEN_RIGHTBRACE)
                    braces--;
                forward();
        }
        
        function_definition(name, start, params,param_count);
    }

    Value if_stmt()
    {
        consume(TOKEN_IF, "if error");
        Value val_ret;
        val_ret.type = VAR_NULL;
        Value cond_val = expression();
        int cond = is_truthy(cond_val);

        
        consume(TOKEN_LEFTBRACE, "{ error");

        int executed = 0;

        if (cond != 0)
        {
            // Execute if block
        return block();
            executed = 1;
        }
        else
        {
            skip_block();
        }

        while (current_token.type == TOKEN_ELIF)
        {
            forward();
            Value elif_cond_val = expression();
            int elif_cond = is_truthy(elif_cond_val);
            consume(TOKEN_LEFTBRACE, "{ error");
            if (!executed && elif_cond != 0)
            {
            return  block();
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
            return block();
            }
            else
            {
                skip_block();
            }
        }
        return val_ret;
    }

    void while_stmt(void)
    {
        int after_cond = indx;
        int after_while = -1;
        forward(); // while
        Value cond_val = expression();
        consume(TOKEN_LEFTBRACE, "Error {");
        int braces = 1;
        while (braces > 0)
        {
        if (current_token.type == TOKEN_LEFTBRACE)
            braces++;
        else if (current_token.type == TOKEN_RIGHTBRACE)
            braces--;
        forward();
        }
        after_while = indx;

        while (1)
        {
            indx = after_cond;
            forward();
            cond_val = expression();
            int cond = is_truthy(cond_val);

            consume(TOKEN_LEFTBRACE, "Error {");
            if (cond != 0)
            {
            block();
            }
            else
            {
                skip_block();
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

    Value block()
    {
        Token token = current_token;
        Value ret_val;
        ret_val.type = VAR_NULL;
        int braces = 1;
        while (braces != 0 && current_token.type != TOKEN_EOF)
        {
            if (current_token.type == TOKEN_PRINT)
                print_stmt();
            else if (current_token.type == TOKEN_ID)
                assignation();
            else if (current_token.type == TOKEN_IF)
            {
                ret_val = if_stmt();
               if (ret_val.type != VAR_NULL)
                return ret_val;
            }
            else if (current_token.type == TOKEN_WHILE)
                while_stmt();
            else if (current_token.type == TOKEN_RETURN){
                forward();
                ret_val = expression();
                forward(); // consumir la '}' final
                return ret_val;
            }
            else if (current_token.type == TOKEN_LEFTBRACE)
                        braces++;
            else if (current_token.type == TOKEN_RIGHTBRACE)
                braces--;

            else
            {
                syntax_error("Invalid sentence\n", token);
                exit(1);
            }
        }

        consume(TOKEN_RIGHTBRACE, "error }");
        
        return ret_val;
    }

    Value comparison_expr()
    {
        Value left = arith_expr();
        while (current_token.type == TOKEN_EQ || current_token.type == TOKEN_NE ||
            current_token.type == TOKEN_LT || current_token.type == TOKEN_GT ||
            current_token.type == TOKEN_LE || current_token.type == TOKEN_GE)
        {
            TypeToken op = current_token.type;
            int current_line = current_token.line;
            forward();
            Value right = arith_expr();
            left = comparison_op(left, op, right,current_line);
        }
        
        return left;
    }

    Value arith_expr()
    {
        Value left = term();
        while (current_token.type == TOKEN_SUM || current_token.type == TOKEN_REST)
        {



            Token token = current_token;
            TypeToken op = current_token.type;
            forward();
            Value right = term();

            if (left.type == VAR_NULL || right.type == VAR_NULL){
                
                syntax_error("Operation with null",token);
                exit(1);
            }

                if (left.type == VAR_NUMBER && right.type == VAR_NUMBER)
                {
                    if (op == TOKEN_SUM)
                        left.value.num += right.value.num;
                    else
                        left.value.num -= right.value.num;
                }
                else if (left.type == VAR_STRING && right.type == VAR_STRING && op == TOKEN_SUM)
                {
                    char tmp[512];
                    snprintf(tmp, sizeof(tmp), "%s%s", left.value.str, right.value.str);
                    strcpy(left.value.str, tmp);
                }
                else if (left.type == VAR_STRING && right.type == VAR_NUMBER && op == TOKEN_SUM)
                {
                    char tmp[512];
                    snprintf(tmp, sizeof(tmp), "%s%g", left.value.str, right.value.num);
                    strcpy(left.value.str, tmp);
                }
                else if (left.type == VAR_NUMBER && right.type == VAR_STRING && op == TOKEN_SUM)
                {
                    char tmp[512];
                    snprintf(tmp, sizeof(tmp), "%g%s", left.value.num, right.value.str);
                    strcpy(left.value.str, tmp);
                    left.type = VAR_STRING;
                }
                else
                {
                    syntax_error("Error bad Arith", token);
                }
        }
        return left;
    }

    Value comparison_op(Value left, TypeToken op, Value right,int line)
    {
        int result = 0;
        if (left.type == VAR_NUMBER && right.type == VAR_NUMBER)
        {
            double l = left.value.num, r = right.value.num;
            switch (op)
            {
            case TOKEN_EQ:
                result = (l == r);
                break;
            case TOKEN_NE:
                result = (l != r);
                break;
            case TOKEN_LT:
                result = (l < r);
                break;
            case TOKEN_GT:
                result = (l > r);
                break;
            case TOKEN_LE:
                result = (l <= r);
                break;
            case TOKEN_GE:
                result = (l >= r);
                break;
            default:
            {

                syntax_error_line("Wrong operation",line);
            }
            }
        }
        else if (left.type == VAR_STRING && right.type == VAR_STRING)
        {
            int cmp = strcmp(left.value.str, right.value.str);
            switch (op)
            {
            case TOKEN_EQ:
                result = (cmp == 0);
                break;
            case TOKEN_NE:
                result = (cmp != 0);
                break;
            case TOKEN_LT:
                result = (cmp < 0);
                break;
            case TOKEN_GT:
                result = (cmp > 0);
                break;
            case TOKEN_LE:
                result = (cmp <= 0);
                break;
            case TOKEN_GE:
                result = (cmp >= 0);
                break;
            default:
            {

                syntax_error_line("Wrong operation",line);
            }
            }
        }
        else if (left.type == VAR_NULL && right.type == VAR_NULL){
            switch (op)
            {
            case TOKEN_EQ:
                result = 1;
                break;
            case TOKEN_NE:
                result = 0;
                break;
            default:
                syntax_error_line("Wrong operation with null", line);
            }
        }
        else if (left.type == VAR_NULL || right.type == VAR_NULL)
        {
            switch (op)
            {
            case TOKEN_EQ:
                result = 0;
                break;
            case TOKEN_NE:
                result = 1;
                break;
            default:
                syntax_error_line("Wrong operation with null", line);
            }
        }
        else
        {
            switch (op)
            {
            case TOKEN_EQ:
                result = 0;
                break;
            case TOKEN_NE:
                result = 1;
                break;
            default:
            {

                syntax_error_line("Wrong operation", line);
            }
            }
        }
        Value v;
        v.type = VAR_NUMBER;
        v.value.num = result;
        return v;
    }

    Value term()
    {
        Value left = factor();
        while (current_token.type == TOKEN_MUL || current_token.type == TOKEN_DIV)
        {
            Token token = current_token;
            TypeToken op = current_token.type;
            forward();
            Value right = factor();

            if (left.type == VAR_NULL || right.type == VAR_NULL)
            {
                syntax_error("Operation with null", token);
                exit(1);
            }

            if (left.type == VAR_NUMBER && right.type == VAR_NUMBER)
            {
                if (op == TOKEN_MUL)
                    left.value.num *= right.value.num;
                else
                {
                    double divisor = right.value.num;
                    if (divisor == 0)
                    {
                        syntax_error("Division by zero", token);
                        exit(1);
                    }
                    left.value.num /= right.value.num;
                }
            }
        }
        return left;
    }

        Value factor()
        { // Operator Prefix ++ --
            Value v;
            v.type = VAR_NUMBER;

            if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
            {
                Token token = current_token;
                TypeToken op = current_token.type;
                forward();
                if (current_token.type != TOKEN_ID)
                {
                    syntax_error("Syntax Error", token);
                    exit(1);
                }

                char name[64];
                strcpy(name, current_token.name);
                Value old = getVarValue(name);
                if (old.type != VAR_NUMBER)
                {
                    syntax_error("Bad operation", token);
                }
                double new_val = (op == TOKEN_INC) ? old.value.num + 1 : old.value.num - 1;
                assignNumberVar(name, new_val);
                forward();
                v.type = VAR_NUMBER;
                v.value.num = new_val;
                return v;
            }
            else if (current_token.type == TOKEN_NOT)
            {
                forward();
                Value operand = factor(); 
                int truthy = is_truthy(operand);
                Value v;
                v.type = VAR_NUMBER;
                v.value.num = truthy ? 0 : 1;
                return v;
            }

            else if (current_token.type == TOKEN_TRUE)
            // Boolean
            {
                forward();
                v.value.num = 1;
                return v;
            }else if (current_token.type == TOKEN_FALSE)
            {
                forward();
                v.value.num = 0;
                return v;
            }
            // Number
            else if (current_token.type == TOKEN_NUM)
            {
                v.value.num = current_token.value;
                forward();
                return v;
            }
            else if (current_token.type == TOKEN_ID)
            {
                char name[64];
                strcpy(name, current_token.name);
                forward();

                if (current_token.type == TOKEN_PARENTLEFT)
                {
                    forward(); // Consume TOKEN (
                    Value v[100];
                    int count = 0;

                    while (current_token.type != TOKEN_PARENTRIGHT)
                    {
                        v[count] = expression();
                        count++;
                    
                    }
                    int last_index = indx;
                    consume(TOKEN_PARENTRIGHT, "expected )");
                    Value v_r = function_call(name, v, count, last_index);
                    return v_r;
                }

                if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
                {
                    TypeToken op = current_token.type;
                    forward();
                    Value old = getVarValue(name);
                    double new_val = (op == TOKEN_INC) ? old.value.num + 1 : old.value.num - 1;
                    assignNumberVar(name, new_val);
                    v.value.num = old.value.num;
                    return v;
                }
                v = getVarValue(name);
                return v;
            }
            else if (current_token.type == TOKEN_PARENTLEFT)
            {
                forward();
                v = expression();
                consume(TOKEN_PARENTRIGHT, "expected )");
                return v;
            }
            else if (current_token.type == TOKEN_STRING)
            {

                strcpy(v.value.str, current_token.name);
                v.type = VAR_STRING;
                forward();
                return v;
            }
            else if (current_token.type == TOKEN_NULL)
            {
                v.type = VAR_NULL;
                forward();
                return v;
            }
            else
            {
                syntax_error("Syntax Error", current_token);
                exit(1);
            }
        }

        void assign_compound(char *name, TypeToken op, double val, int op_line)
        {
            Value v = getVarValue(name);
            double current = v.value.num;
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
                    syntax_error_line("Division by zero", op_line);
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
            Token token = current_token;
            TypeToken op = current_token.type;

            forward();
            if (current_token.type != TOKEN_ID)
            {
                syntax_error("",token);
                exit(1);
            }
            strcpy(name, current_token.name);
            forward();
            Value v = getVarValue(name);
            double old = v.value.num;
            double new_val = (op == TOKEN_INC) ? old + 1 : old - 1;
            assignNumberVar(name, new_val);
            return;
        }

        if (current_token.type == TOKEN_ID)
        {
            strcpy(name, current_token.name);
            if (peek_next_token_type() == TOKEN_PARENTLEFT){ //is a function
                factor();
                return;
            }

            forward(); //Consume ID
            
            if (current_token.type == TOKEN_INC || current_token.type == TOKEN_DEC)
            {
                TypeToken op = current_token.type;
                forward();
                Value v = getVarValue(name);
                double old = v.value.num;
                double new_val = (op == TOKEN_INC) ? old + 1 : old - 1;
                assignNumberVar(name, new_val);
                return;
            }

        if (current_token.type == TOKEN_PLUS_ASSIGN ||
            current_token.type == TOKEN_MINUS_ASSIGN ||
            current_token.type == TOKEN_DIV_ASSIGN ||
            current_token.type == TOKEN_MULT_ASSIGN)
        {
            TypeToken op = current_token.type;
            double current_line = current_token.line;
            forward();
            Value val = expression();
            assign_compound(name, op, val.value.num,current_line);
            return;
        }

        consume(TOKEN_ASIGN, "not found '='");
        Value val = expression();
        setVariable(name,val);
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

            if (next == TOKEN_PARENTLEFT)
            { // is a function
                Value val = factor();
                snprintf(temp, sizeof(temp), "%g", val.value.num);
                strncat(buffer, temp, buffsize - strlen(buffer) - 1);
            }
            else if (next == TOKEN_INC || next == TOKEN_DEC)
            {
                Value val = factor();
                snprintf(temp, sizeof(temp), "%g", val.value.num);
                strncat(buffer, temp, buffsize - strlen(buffer) - 1);
            }
            else
            {
                forward();
                int found = 0;
                Scope sc = scope_stack[scope_depth];

                for (int i = 0; i < sc.num_vars; i++)
                {
                    if (strcmp(sc.vars[i].name, var_name) == 0)
                    {
                        if (sc.vars[i].type == VAR_STRING)
                        {
                            strncat(buffer, sc.vars[i].value.str, buffsize - strlen(buffer) - 1);
                        }
                        else if (sc.vars[i].type == VAR_NULL)
                        {
                            strncat(buffer, "null", buffsize - strlen(buffer) - 1);
                        }
                        else
                        {
                            snprintf(temp, sizeof(temp), "%g", sc.vars[i].value.num);
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
        else if (current_token.type == TOKEN_NULL){
            snprintf(temp, sizeof(temp), "null");
            strncat(buffer, temp, buffsize - strlen(buffer) - 1);
        }
        else
        {
            Value val = factor();
            snprintf(temp, sizeof(temp), "%g", val.value.num);
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

        fwrite(buffer, 1, strlen(buffer), stdout);
        printf("\n");
    }

    Value function_call(char* name,Value args_v[],int count,int last_index)
    {
        pushScope();
        Value func_val = getVarValue(name);
        func_val.type = VAR_NULL;
            if (count < func_val.func.param_count)
            {
                syntax_error_line("Few params in function",current_token.line);
            }
            else if (count > func_val.func.param_count)
            {
                syntax_error_line("Too much params in function", current_token.line);
            }

            for (int x = 0; x < count; x++)
            {
                setVariable(func_val.func.param[x],args_v[x]);
            }
        indx = func_val.func.start;
        forward(); //Update position on first token in function
        Value ret_val = block();
        indx = last_index;   
        forward();
        popScope();
        return ret_val;
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
            else if (current_token.type == TOKEN_FUNC)
            {
                defineNewFunction();
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
                syntax_error("invalid statement", current_token);
                exit(1);
            }
        }
    }