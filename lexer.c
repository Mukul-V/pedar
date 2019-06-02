#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>

#include "types.h"
#include "utils.h"
#include "array.h"
#include "table.h"

#include "lexer.h"

token_t *
token_create(long64_t key, long64_t value, long64_t pos, long64_t row, long64_t col)
{
    token_t *token = (token_t *)malloc(sizeof(token_t));
    token->key = key;
    token->value = value;
    token->pos = pos;
    token->row = row;
    token->col = col;
    return token;
}

long64_t
token_destroy(itable_t *it)
{
    token_t *token = (token_t *)it->value;
    if(token){
        if(token->value){
            free((void *)token->value);
        }
        free(token);
    }
    free(it);
    return 1;
}

void
lexer_error(const char *source, long64_t pos, long64_t row, long64_t col, char *str){
    printf("lexer(%lld:%lld): %s!\n", row, col, str);
    char c;
    while((c = source[pos--])){
        if(c == '\n'){
            break;
        }
    }
    while((c = source[pos++])){
        if(c != '\n'){
            printf("%c", c);
        }
    }
    exit(-1);
}


void
lexer(table_t *ls, const char *source)
{
    long64_t c, a;
    long64_t pos = 0;
    long64_t row = 1;
    long64_t col = 1;

    while ((c = source[pos])) {
        if(c == '\n' || c == '\v'){
            row++;
            pos++;
            col = 1;
            continue;
        }

        if(white_space(c)){
            pos++;
            col++;
            continue;
        }

        if(!valid_alpha(c) && !valid_digit(c) && c != '_'){
            if(c == '"' || c == '\''){
                // parse string literal, currently, the only supported escape
                // character is '\n', store the string literal into data.
                long64_t pos2 = pos + 1;
                pos++;
                col++;

                while ((a = source[pos]) && a != c) {
                    pos++;
                    col++;
                }

                char *data = malloc(sizeof(char) * (pos - pos2));
                strncpy(data, source + pos2, pos - pos2 );
                data[pos - pos2] = '\0';

                if(table_rpush(ls, (value_p)token_create(TOKEN_DATA,(value_t)data,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }

                pos++;
                col++;
                continue;
            }
            else if(c == '('){
                if(table_rpush(ls, (value_p)token_create(TOKEN_LPAREN,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == ')'){
                if(table_rpush(ls, (value_p)token_create(TOKEN_RPAREN,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '['){
                if(table_rpush(ls, (value_p)token_create(TOKEN_LBRACKET,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == ']'){
                if(table_rpush(ls, (value_p)token_create(TOKEN_RBRACKET,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '{'){
                if(table_rpush(ls, (value_p)token_create(TOKEN_LBRACE,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '}'){
                if(table_rpush(ls, (value_p)token_create(TOKEN_RBRACE,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '='){
                if ((a = source[pos + 1]) && a == '=') {
                    if(table_rpush(ls, (value_p)token_create(TOKEN_EQEQ,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(table_rpush(ls, (value_p)token_create(TOKEN_EQ,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '?'){
                if(table_rpush(ls, (value_p)token_create(TOKEN_QUESTION,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '|'){
                if ((a = source[pos + 1]) && a == '|') {
                    if(table_rpush(ls, (value_p)token_create(TOKEN_LOR,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(table_rpush(ls, (value_p)token_create(TOKEN_OR,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '&'){
                if ((a = source[pos + 1]) && a == '&') {
                    if(table_rpush(ls, (value_p)token_create(TOKEN_LAND,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(table_rpush(ls, (value_p)token_create(TOKEN_AND,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '^'){
                if(table_rpush(ls, (value_p)token_create(TOKEN_CARET,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '!'){
                if ((a = source[pos + 1]) && a == '=') {
                    if(table_rpush(ls, (value_p)token_create(TOKEN_NEQ,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(table_rpush(ls, (value_p)token_create(TOKEN_NOT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '<'){
                if ((a = source[pos + 1]) && a == '<') {
                    if(table_rpush(ls, (value_p)token_create(TOKEN_LTLT,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if ((a = source[pos + 1]) && a == '=') {
                    if(table_rpush(ls, (value_p)token_create(TOKEN_LTEQ,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(table_rpush(ls, (value_p)token_create(TOKEN_LT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '>'){
                if ((a = source[pos + 1]) && a == '>') {
                    if(table_rpush(ls, (value_p)token_create(TOKEN_GTGT,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if ((a = source[pos + 1]) && a == '=') {
                    if(table_rpush(ls, (value_p)token_create(TOKEN_GTEQ,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(table_rpush(ls, (value_p)token_create(TOKEN_GT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '+'){
                if(table_rpush(ls, (value_p)token_create(TOKEN_PLUS,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '-'){
                if ((a = source[pos + 1]) && a == '>') {
                    if(table_rpush(ls, (value_p)token_create(TOKEN_MINUSGT,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(table_rpush(ls, (value_p)token_create(TOKEN_MINUS,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '*'){
                if(table_rpush(ls, (value_p)token_create(TOKEN_STAR,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '/'){
                if ((a = source[pos + 1]) && a == '/') {
                    while ((a = source[pos]) && a != '\n') {
                        pos++;
                        col++;
                    }
                    row++;
                    continue;
                }
                else if ((a = source[pos + 1]) && a == '*') {
                    pos += 2;
                    col += 2;
                    long64_t i = 0, p = 0;
                    while ((a = source[pos])) {
                        if(a == '*'){
                            if ((a = source[pos + 1]) && a == '/' && i < 1) {
                                break;
                            }
                            i--;
                        }
                        if(a == '\n'){
                            row++;
                        }
                        if(a == '/'){
                            if ((p = source[pos + 1]) && p == '*') {
                                i++;
                            }
                        }
                        pos++;
                        col++;
                    }
                    continue;
                }
                else if(table_rpush(ls, (value_p)token_create(TOKEN_SLASH,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '%'){
                if(table_rpush(ls, (value_p)token_create(TOKEN_PERCENT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '.'){
                if(table_rpush(ls, (value_p)token_create(TOKEN_DOT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == ','){
                if(table_rpush(ls, (value_p)token_create(TOKEN_COMMA,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == ':'){
                if(table_rpush(ls, (value_p)token_create(TOKEN_COLON,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == ';'){
                if(table_rpush(ls, (value_p)token_create(TOKEN_SEMICOLON,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '~'){
                if(table_rpush(ls, (value_p)token_create(TOKEN_TILDE,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '#'){
                if(table_rpush(ls, (value_p)token_create(TOKEN_HASH,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '_'){
                if(table_rpush(ls, (value_p)token_create(TOKEN_UNDERLINE,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '@'){
                if(table_rpush(ls, (value_p)token_create(TOKEN_AT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '$'){
                if(table_rpush(ls, (value_p)token_create(TOKEN_DOLLER,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else {
                if(c < 0){
                    break;
                }
                lexer_error(source, pos, row, col, "bad token!");
            }
        }
        else if(valid_digit(c)) {
            // parse number, three kinds: dec(123) hex(0x123) oct(017)
            long64_t token_val = c - '0';
            long64_t pos2 = pos;

            if (token_val > 0) {
                pos++;
                col++;

                // dec, starts with [1-9]
                while ((a = source[pos]) && (valid_digit(a) || a == '.') && !white_space(a)) {
                    pos++;
                    col++;
                }

                char *data = malloc(sizeof(char) * (pos - pos2));
                strncpy(data, source + pos2, pos - pos2 );
                data[pos - pos2] = '\0';

                if(table_rpush(ls, (value_p)token_create(TOKEN_NUMBER, (long64_t)data,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }

                continue;
            } else {
                // starts with number 0
                a = source[pos + 1];
                if (a == 'x' || a == 'X') {
                    pos+=2;
                    col+=2;
                    //hex
                    while ((a = source[pos]) && valid_hexadecimal(a)) {
                        token_val = token_val * 16 + (a & 15) + (a >= 'A' ? 9 : 0);
                        pos++;
                        col++;
                    }

                    char *data = malloc(sizeof(char) * (pos - pos2));
                    strncpy(data, source + pos2, pos - pos2 );
                    data[pos - pos2] = '\0';

                    if(table_rpush(ls, (value_p)token_create(TOKEN_NUMBER ,(long64_t)data ,pos2 ,row ,col-(pos-pos2))) == nullptr){
                        lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                    }
                    continue;
                } else if(valid_digit(a)){
                    // oct
                    while ((a = source[pos]) && valid_octal(a)) {
                        token_val = token_val * 8 + a - '0';
                        pos++;
                        col++;
                    }

                    char *data = malloc(sizeof(char) * (pos - pos2));
                    strncpy(data, source + pos2, pos - pos2 );
                    data[pos - pos2] = '\0';

                    if(table_rpush(ls, (value_p)token_create(TOKEN_NUMBER, (long64_t)data,pos2,row,col-(pos-pos2))) == nullptr){
                        lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                    }
                } else {
                    pos++;
                    col++;
                }
            }

            char *data = malloc(sizeof(char) * (pos - pos2));
            strncpy(data, source + pos2, pos - pos2 );
            data[pos - pos2] = '\0';

            if(table_rpush(ls, (value_p)token_create(TOKEN_NUMBER, (long64_t)data,pos2,row,col-(pos-pos2))) == nullptr){
                lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
            }
            continue;
        }
        else {
            // parse identifier
            long64_t pos2 = pos;
            long64_t hash = 0;
            while ((a = source[pos]) && ( valid_alpha(a) || valid_digit(a) || (a == '_'))) {
                hash = hash * 147 + a;
                pos++;
                col++;
            }

            if(strncmp(source + pos2, "while", 5) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_WHILE,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "if", 2) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_IF,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "else", 4) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_ELSE,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "break", 5) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_BREAK,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "continue", 8) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_CONTINUE,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "this", 4) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_THIS,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "super", 5) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_SUPER,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "return", 6) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_RETURN,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "import", 6) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_IMPORT,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "sizeof", 6) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_SIZEOF,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "typeof", 6) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_TYPEOF,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "format", 6) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_FORMAT,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "print", 5) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_PRINT,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "delete", 6) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_DELETE,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "null", 4) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_NULL,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "ref", 3) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_REF,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "count", 4) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_COUNT,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "__open__", 8) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_OPEN,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "__read__", 8) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_READ,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "__seek__", 8) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_SEEK,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "__write__", 9) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_WRITE,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "__close__", 9) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_CLOSE,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "rename", 6) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_RENAME,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "cwd", 3) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_CWD,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "chdir", 5) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_CHDIR,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "getc", 4) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_GETC,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "gets", 4) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_GETS,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "tick", 4) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_TICK,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "time", 4) == 0){
                if(table_rpush(ls, (value_p)token_create(TOKEN_TIME,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            }

            char *var = malloc(sizeof(char) * (pos - pos2));
            strncpy(var, source + pos2, pos - pos2);
            var[pos - pos2] = '\0';

            if(table_rpush(ls, (value_p)token_create(TOKEN_ID,(value_t)var,pos2,row,col-(pos-pos2))) == nullptr){
                lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
            }
            continue;
        }

        pos++;
        col++;
    }
}
