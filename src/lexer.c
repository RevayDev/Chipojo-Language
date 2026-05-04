#include "lexer.h"
char *input = NULL;
int indx = 0;

void nextChar(void) { indx++; }
char currentChar(void) { return input[indx]; }

void jumpBlankspace(void)
{
    while (isspace((unsigned char)currentChar()))
        nextChar();
}

void jumpBOM(void)
{
    if ((unsigned char)input[0] == 0xEF && (unsigned char)input[1] == 0xBB && (unsigned char)input[2] == 0xBF)
    {
        indx = 3;
    }
}

TypeToken peek_next_token_type(void)
{
    int saved_pos = indx;
    Token t = nextToken();
    TypeToken type = t.type;
    indx = saved_pos;
    return type;
}

Token nextToken(){
    jumpBlankspace();
    Token t;
    char c = currentChar();
    if (c == '\0')
    {
        t.type = TOKEN_EOF;
        return t;
    }
    //var and identificator
    if (isalpha((unsigned char) c)){
        int i = 0;
        while (isalnum((unsigned char)currentChar()) && i < 63)
        {
            t.name[i++] = currentChar();
            nextChar();
        }
        t.name[i] = '\0';

        if (strcmp(t.name, "print") == 0){
            t.type = TOKEN_PRINT;
        }
        else{
            t.type = TOKEN_ID;
        }
        return t;
    }

    // Number
    if (isdigit((unsigned char)c))
    {
        t.value = 0;
        while (isdigit((unsigned char)currentChar()))
        {
            t.value = t.value * 10 + (currentChar() - '0');
            nextChar();
        }
        t.type = TOKEN_NUM;
        return t;
    }

    //String
    if (c == '"'){
        nextChar();
        int i = 0;
        while (isalnum((unsigned char)currentChar()) && i < 63)
        {
            t.name[i++] = currentChar();
            nextChar();
        }
        if (currentChar() == '"')
        {
            nextChar();
            t.name[i] = '\0';
            t.type = TOKEN_STRING;
        }
        else
        {
            t.type = TOKEN_ERROR;
            printf("Lizzard Error:String unclosed\n");
        }
        return t;
    }

    //sign
    switch (c)
    {
    case '=':
        t.type = TOKEN_ASIGN;
        break;
    case '(':
        t.type = TOKEN_LEFTPARENT;
        break;
    case ')':
        t.type = TOKEN_RIGHTPARENT;
        break;

    default:
        nextChar();
        return nextToken();
    }
    nextChar();
    return t;
}