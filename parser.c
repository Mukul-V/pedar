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
#include "parser.h"

void
parser_error(token_t *token, char *str)
{
    printf("parser(%lld:%lld): \'%c\' %lld %s!\n",
        token->row, token->col, (char)token->key, token->key, str);
    exit(-1);
}

class_t *
parser_class_get(class_t *clspar, char *key)
{
    if(!clspar){
        return clspar;
    }

    if(strncmp(key, clspar->key, max(strlen(key), strlen(clspar->key))) == 0){
		return clspar;
	}

	itable_t *u;

	for(u = clspar->childrens->begin; u && (u != clspar->childrens->end); u = u->next){
		class_t *cls = (class_t *)u->value;
		if(!((char*)cls->key)){
    		continue;
		}

		if(strncmp(key, cls->key, max(strlen(key), strlen(cls->key))) == 0){
			return cls;
		}
	}

	class_t *clsres;
	for(u = clspar->parents->begin; u && (u != clspar->parents->end); u = u->next){
		class_t *cls = (class_t *)u->value;
		if(!(clsres = parser_class_get(cls, key))){
			continue;
		}
	  	return clsres;
	}

	return nullptr;
}

#include "libs/io.h"
#include "libs/file.h"
#include "libs/keyboard.h"
#include "libs/time.h"

itable_t *
expression(table_t *tls, itable_t *c, array_t *code)
{
    while((c != tls->end)){
        token_t *token = (token_t *) c->value;

        /*printf("+token(%lld:%lld): \'%c\' %lld!\n",
            token->row, token->col, (char)token->key, token->key);*/

        if(token->key == TOKEN_SUPER){
            c = c->next;
            token = (token_t *) c->value;
            array_rpush(code, SUPER);
            if(token->key == TOKEN_LPAREN) {
                array_rpush(code, SVPA);
                array_rpush(code, NEW);

                /*
                call function:
                push {parameter_counter, parameters}
                */
                long64_t cnt = 0;

                c = c->next;
                token = (token_t *) c->value;

                while(token->key != TOKEN_RPAREN){
                    c = expression(tls, c, code);
                    array_rpush(code, PUSH);
                    token = (token_t *) c->value;
                    cnt++;

                    if(token->key != TOKEN_RPAREN){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                array_rpush(code, IMM);
                array_rpush(code, (value_t)cnt);
                array_rpush(code, TP_IMM);
                array_rpush(code, PUSH);

                array_rpush(code, CALL);

                c = c->next;
                continue;
            }
            continue;
        } else if(token->key == TOKEN_THIS){
            c = c->next;
            token = (token_t *) c->value;
            array_rpush(code, THIS);
            if(token->key == TOKEN_LPAREN) {
                array_rpush(code, SVPA);
                array_rpush(code, NEW);

                /*
                call function:
                push {parameter_counter, parameters}
                */
                long64_t cnt = 0;

                c = c->next;
                token = (token_t *) c->value;

                while(token->key != TOKEN_RPAREN){
                    c = expression(tls, c, code);
                    array_rpush(code, PUSH);
                    token = (token_t *) c->value;
                    cnt++;
                    if(token->key != TOKEN_RPAREN){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                array_rpush(code, IMM);
                array_rpush(code, (value_t)cnt);
                array_rpush(code, TP_IMM);
                array_rpush(code, PUSH);

                array_rpush(code, CALL);
                c = c->next;
                continue;
            }
            continue;
        } else if(token->key == TOKEN_CONTINUE){
            iarray_t *b = array_last(code);
            while(b != code->begin){
                if(b->value == LOPB){
                    array_rpush(code, JMP);
                    array_rpush(code, (value_t)b);
                    break;
                }
                b = b->previous;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_BREAK){
            array_rpush(code, BREAK);
            c = c->next;
            continue;
        } else if(token->key == TOKEN_ID){
            array_rpush(code, VAR);
            array_rpush(code, (value_t)token->value);
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN) {
                array_rpush(code, SVPA);
                array_rpush(code, NEW);

                /*
                call function:
                push {parameter_counter, parameters}
                */
                long64_t cnt = 0;

                c = c->next;
                token = (token_t *) c->value;

                while(token->key != TOKEN_RPAREN){
                    c = expression(tls, c, code);
                    array_rpush(code, PUSH);
                    token = (token_t *) c->value;
                    cnt++;

                    if(token->key != TOKEN_RPAREN){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                array_rpush(code, IMM);
                array_rpush(code, (value_t)cnt);
                array_rpush(code, TP_IMM);
                array_rpush(code, PUSH);

                array_rpush(code, CALL);

                c = c->next;
                token = (token_t *) c->value;
            }
            continue;
        } else if(token->key == TOKEN_DATA){
            array_rpush(code, IMM);
            array_rpush(code, (value_t)token->value);
            array_rpush(code, TP_DATA);
            c = c->next;
            continue;
        } else if(token->key == TOKEN_NUMBER){
            array_rpush(code, IMM);
            array_rpush(code, (value_t)token->value);
            array_rpush(code, TP_NUMBER);
            c = c->next;
            continue;
        } else if(token->key == TOKEN_NULL){
            array_rpush(code, IMM);
            array_rpush(code, (value_t)token->value);
            array_rpush(code, TP_NULL);
            c = c->next;
            continue;
        } else if(token->key == TOKEN_EQ){
            array_rpush(code, PUSH);

            c = c->next;
            token = (token_t *) c->value;
            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            array_rpush(code, SD);
            token = (token_t *) c->value;
            continue;
        } else if(token->key == TOKEN_NOT){
            // emit code, use <expr> == 0
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, PUSH);
            array_rpush(code, IMM);
            array_rpush(code, 0);
            array_rpush(code, TP_IMM);
            array_rpush(code, EQ);
            continue;
        } else if(token->key == TOKEN_TILDE){
            // emit code, use <expr> XOR -1
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, PUSH);
            array_rpush(code, IMM);
            array_rpush(code, -1);
            array_rpush(code, TP_IMM);
            array_rpush(code, XOR);
            continue;
        } else if(token->key == TOKEN_QUESTION){
            array_rpush(code, PUSH);
            // expr ? expr : expr;
            array_rpush(code, JZ);
            iarray_t *a = array_rpush(code, 0);
            c = c->next;
            c = expression(tls, c, code);
            token = (token_t *) c->value;
            if (token->key != TOKEN_COLON) {
                parser_error(token, "missing colon in conditional!");
            }
            c = c->next;
            array_rpush(code, JMP);
            iarray_t *b = array_rpush(code, 0);

            c = expression(tls, c, code);

            a->value = (long64_t) b->next;
            b->value = (long64_t) array_rpush(code, NUL);
            continue;
        } else if(token->key == TOKEN_CARET){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, XOR);
            continue;
        } else if(token->key == TOKEN_STAR){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, MUL);
            continue;
        } else if(token->key == TOKEN_SLASH){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, DIV);
            continue;
        } else if(token->key == TOKEN_BACKSLASH){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, DIV);
            continue;
        } else if(token->key == TOKEN_PERCENT){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, MOD);
            continue;
        } else if(token->key == TOKEN_PLUS){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, ADD);
            continue;
        } else if(token->key == TOKEN_MINUS){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, SUB);
            continue;
        } else if(token->key == TOKEN_EQEQ){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, EQ);
            continue;
        } else if(token->key == TOKEN_NEQ){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, NE);
            continue;
        } else if(token->key == TOKEN_REF){
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, REF);
            continue;
        } else if(token->key == TOKEN_GTEQ){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, GE);
            continue;
        } else if(token->key == TOKEN_LTEQ){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, LE);
            continue;
        } else if(token->key == TOKEN_LAND){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, LAND);
            continue;
        } else if(token->key == TOKEN_LOR){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, LOR);
            continue;
        } else if(token->key == TOKEN_AND){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, AND);
            continue;
        } else if(token->key == TOKEN_OR){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, OR);
            continue;
        } else if(token->key == TOKEN_LT){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, LT);
            continue;
        } else if(token->key == TOKEN_GT){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, GT);
            continue;
        } else if(token->key == TOKEN_GTGT){
        array_rpush(code, PUSH);
        c = c->next;
        c = expression(tls, c, code);
        array_rpush(code, RSHIFT);
        continue;
        } else if(token->key == TOKEN_LTLT){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, LSHIFT);
            continue;
        } else if(token->key == TOKEN_PLUS){
            array_rpush(code, PUSH);
            c = c->next;
            c = expression(tls, c, code);
            array_rpush(code, ADD);
            continue;
        } else if(token->key == TOKEN_MINUSGT){
            array_rpush(code, RAR);
            c = c->next;
            continue;
        } else if(token->key == TOKEN_SIZEOF){
            c = c->next;
            token = (token_t *) c->value;

            if(token->key != TOKEN_LPAREN){
                parser_error(token, "expression in 'sizeof' must start with '('!");
            }

            c = c->next;
            token = (token_t *) c->value;

            if(token->key != TOKEN_RPAREN){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
            }

            if(token->key != TOKEN_RPAREN){
                parser_error(token, "expression in 'sizeof' must end with ')'!");
            }

            array_rpush(code, SIZEOF);

            c = c->next;
            token = (token_t *) c->value;
            continue;
        } else if(token->key == TOKEN_COUNT){
            c = c->next;
            token = (token_t *) c->value;

            if(token->key != TOKEN_LPAREN){
                parser_error(token, "expression in 'count' must start with '('!");
            }

            c = c->next;
            token = (token_t *) c->value;

            if(token->key != TOKEN_RPAREN){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
            }

            if(token->key != TOKEN_RPAREN){
                parser_error(token, "expression in 'count' must end with ')'!");
            }

            array_rpush(code, COUNT);

            c = c->next;
            token = (token_t *) c->value;
            continue;
        } else if(token->key == TOKEN_TYPEOF){
            c = c->next;
            token = (token_t *) c->value;

            if(token->key != TOKEN_LPAREN){
                parser_error(token, "expression in 'typeof' must start with '('!");
            }

            c = c->next;
            token = (token_t *) c->value;

            if(token->key != TOKEN_RPAREN){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
            }

            if(token->key != TOKEN_RPAREN){
                parser_error(token, "expression in 'typeof' must end with ')'!");
            }

            array_rpush(code, TYPEOF);

            c = c->next;
            token = (token_t *) c->value;
            continue;
        } else if(token->key == TOKEN_IF){

            c = c->next;
            token = (token_t *) c->value;

            if(token->key != TOKEN_LPAREN){
                parser_error(token, "expression in 'if' must start with '('!");
            }

            c = c->next;
            token = (token_t *) c->value;

            while(token->key != TOKEN_RPAREN){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_RPAREN){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }

            c = c->next;
            token = (token_t *) c->value;

            if(token->key != TOKEN_LBRACE){
                parser_error(token, "statement in 'if' must start with '{'!");
            }

            array_rpush(code, JZ);
            iarray_t *a = array_rpush(code, 0);

            c = c->next;
            token = (token_t *) c->value;

            while(token->key != TOKEN_RBRACE){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_RBRACE){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }

            c = c->next;
            token = (token_t *) c->value;

            if(token->key == TOKEN_ELSE){
                c = c->next;
                token = (token_t *) c->value;

                array_rpush(code, JMP);
                iarray_t *b = array_rpush(code, NUL);

                if(token->key != TOKEN_LBRACE){
                    a->value = (long64_t) array_rpush(code, NUL);
                    b->value = a->value;
                    continue;
                }

                c = expression(tls, c, code);

                c = c->next;
                token = (token_t *) c->value;

                a->value = (long64_t) b->next;
                a = b;
            }

            a->value = (long64_t) array_rpush(code, NUL);
            continue;
        } else if(token->key == TOKEN_WHILE){
            c = c->next;

            iarray_t *a = array_rpush(code, LOPB);

            /* current token must be '(' next it then call c = expression */
            c = c->next;

            while(token->key != TOKEN_RPAREN){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_RPAREN){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }

            /* return token ')' */
            c = c->next;

            array_rpush(code, JZ);
            iarray_t *b = array_rpush(code, 0);

            /* current token must be '{' next it then call statement */
            c = c->next;

            while(token->key != TOKEN_RBRACE){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_RBRACE){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }

            /* return token '}' */
            c = c->next;

            array_rpush(code, JMP);
            array_rpush(code, (long64_t)a);

            b->value = (value_t) array_rpush(code, LOPE);
            continue;
        } else if(token->key == TOKEN_RETURN){
            c = c->next;
            /*
            'return expr;'
            then continue token then equal with simicolon
            */
            token = (token_t *) c->value;
            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            /* return token simicolon */
            array_rpush(code, RET);
            array_rpush(code, LEV);

            token = (token_t *) c->value;
            if(token->key != TOKEN_SEMICOLON){
                parser_error(token, "using semicolon in end of definition on return struction!");
            }
            continue;
        } else if(token->key == TOKEN_PRINT){
            c = c->next;
            token = (token_t *) c->value;

            if(token->key != TOKEN_LPAREN){
                parser_error(token, "print must end to '('!");
            }

            c = c->next;
            token = (token_t *) c->value;

            long64_t i = 0;
            while(token->key != TOKEN_RPAREN){
                c = expression(tls, c, code);
                array_rpush(code, PUSH);
                i++;
                token = (token_t *) c->value;
                if(token->key != TOKEN_RPAREN){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }

            array_rpush(code, IMM);
            array_rpush(code, i);
            array_rpush(code, TP_IMM);
            array_rpush(code, PUSH);

            array_rpush(code, PRTF);

            if(token->key != TOKEN_RPAREN){
                parser_error(token, "print must end to ')'!");
            }

            c = c->next;
            token = (token_t *) c->value;

            if(token->key != TOKEN_SEMICOLON){
                parser_error(token, "using ';' in end of definition on print!");
            }
            continue;
        } else if(token->key == TOKEN_FORMAT){
            c = c->next;
            token = (token_t *) c->value;

            if(token->key != TOKEN_LPAREN){
                parser_error(token, "format must end to '('!");
            }

            c = c->next;
            token = (token_t *) c->value;

            long64_t i = 0;
            while(token->key != TOKEN_RPAREN){
                c = expression(tls, c, code);
                array_rpush(code, PUSH);
                i++;
                token = (token_t *) c->value;
                if(token->key != TOKEN_RPAREN){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }

            array_rpush(code, IMM);
            array_rpush(code, i);
            array_rpush(code, TP_IMM);
            array_rpush(code, PUSH);

            array_rpush(code, FORMAT);

            if(token->key != TOKEN_RPAREN){
                parser_error(token, "format must end to ')'!");
            }

            c = c->next;
            token = (token_t *) c->value;
            continue;
        } else if(token->key == TOKEN_DELETE){
            c = c->next;
            token = (token_t *) c->value;

            if(token->key != TOKEN_LPAREN){
                parser_error(token, "expression in 'delete' must start with '('!");
            }

            c = c->next;
            token = (token_t *) c->value;

            long64_t i = 0;
            while(token->key != TOKEN_RPAREN){
                c = expression(tls, c, code);
                array_rpush(code, PUSH);
                i++;
                token = (token_t *) c->value;
                if(token->key != TOKEN_RPAREN){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }

            array_rpush(code, IMM);
            array_rpush(code, i - 1);
            array_rpush(code, TP_IMM);
            array_rpush(code, PUSH);

            array_rpush(code, DELETE);

            c = c->next;
            token = (token_t *) c->value;

            continue;
        } else if(token->key == TOKEN_INSERT){
            c = c->next;
            token = (token_t *) c->value;

            if(token->key != TOKEN_LPAREN){
                parser_error(token, "expression in 'insert' must start with '('!");
            }

            c = c->next;
            token = (token_t *) c->value;

            long64_t i = 0;
            while(token->key != TOKEN_RPAREN){
                c = expression(tls, c, code);
                array_rpush(code, PUSH);
                i++;
                token = (token_t *) c->value;
                if(token->key != TOKEN_RPAREN){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }

            array_rpush(code, IMM);
            array_rpush(code, i - 1);
            array_rpush(code, TP_IMM);
            array_rpush(code, PUSH);

            array_rpush(code, INSERT);

            c = c->next;
            token = (token_t *) c->value;

            continue;
        } else if(token->key == TOKEN_LPAREN){
            c = c->next;
            token = (token_t *) c->value;
            while(token->key != TOKEN_RPAREN) {
                c = expression(tls, c, code);
                token = (token_t *) c->value;
                if(token->key != TOKEN_RPAREN){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            token = (token_t *) c->value;
            if(token->key != TOKEN_RPAREN){
                parser_error(token, "must be close paren!");
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_LBRACKET){
            array_rpush(code, SVBR);
            c = c->next;
            token = (token_t *) c->value;
            long64_t cnt = 0;
            while(token->key != TOKEN_RBRACKET){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
                if(token->key == TOKEN_COMMA){
                    array_rpush(code, PUSH);
                    cnt++;
                }
                if(token->key != TOKEN_RBRACKET){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }

            array_rpush(code, PUSH);
            array_rpush(code, DATA);
            array_rpush(code, ++cnt);
            return c;
        } else if(token->key == TOKEN_LBRACE){
            c = c->next;
            token = (token_t *) c->value;
            while(token->key != TOKEN_RBRACE){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
                if(token->key != TOKEN_RBRACE){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            continue;
        } else if(token->key == TOKEN_DOT){
            array_rpush(code, CHG);
            c = c->next;
            continue;
        } else if(token->key == TOKEN_COMMA){
            //array_rpush(code, RST);
            return c;
        } else if(token->key == TOKEN_RPAREN){
            //array_rpush(code, RST);
            return c;
        } else if(token->key == TOKEN_RBRACKET){
        //array_rpush(code, RST);
        return c;
        } else if(token->key == TOKEN_COLON) {
            //array_rpush(code, RST);
            return c;
        } else if(token->key == TOKEN_SEMICOLON) {
            //array_rpush(code, RST);
            return c;
        } else if(token->key == TOKEN_RBRACE) {
            //array_rpush(code, RST);
            return c;
        }

        break;
    }

    token_t *token = (token_t *) c->value;
    parser_error(token, "bad expression!");
    return nullptr;
}

itable_t *
statement(table_t *tls, class_t *base, itable_t *c, class_t *clspar, array_t *code)
{
    while((c != tls->end)){

        token_t *token = (token_t *) c->value;

        /*printf(">token(%lld:%lld): \'%c\' %lld!\n",
        token->row, token->col, (char)token->key, token->key);*/
        /*
        TOKEN_NEQ TOKEN_CARET TOKEN_STAR TOKEN_SLASH TOKEN_BACKSLASH
        TOKEN_PERCENT TOKEN_PLUS TOKEN_MINUS TOKEN_GTEQ TOKEN_LTEQ
        TOKEN_AND TOKEN_OR TOKEN_LT TOKEN_GT TOKEN_LTLT TOKEN_GTGT
        */
        if(token->key == TOKEN_ID){

            char *class_name = (char *)token->value;
            class_t *class_object = parser_class_get(clspar, class_name);

            c = c->next;
            token = (token_t *) c->value;

            if(token->key == TOKEN_DOT){
                parser_error(token, "bad definition of parent class!");
            }
            else
            if(token->key == TOKEN_COLON || token->key == TOKEN_LBRACE){
                class_t *clscur = class_object;
                if(!class_object){
                    clscur = (class_t *)malloc(sizeof(class_t));
                    clscur->key = class_name;
                    clscur->type = CLASS_RAW;

                    clscur->parents = table_create();
                    clscur->childrens = table_create();
                    clscur->variables = table_create();
                    clscur->functions = table_create();

                    clscur->super = clspar;
                }

                table_rpush(clscur->parents, (value_p)clspar);
                table_rpush(clspar->childrens, (value_p)clscur);

                if(token->key == TOKEN_COLON){
                    c = c->next;
                    token = (token_t *) c->value;

                    class_t *scp = nullptr;

                    while(token->key != TOKEN_LBRACE){
                        if(token->key == TOKEN_COMMA){
                            c = c->next;
                            token = (token_t *) c->value;

                            if(token->key != TOKEN_ID){
                                parser_error(token, "bad definition of parent class1!");
                            }

                            if(!(scp = parser_class_get(clspar, (char *)token->value))){
                                if(!(scp = parser_class_get(class_object, (char *)token->value))){
                                    parser_error(token, "bad definition of parent class2!");
                                }
                            }

                            continue;
                        }

                        if(token->key != TOKEN_ID){
                            parser_error(token, "bad definition of parent class!");
                        }

                        if(!(scp = parser_class_get(scp ? scp : clscur, (char *)token->value))){
                            parser_error(token, "bad definition of parent class!");
                        }

                        c = c->next;
                        token = (token_t *) c->value;

                        if(token->key == TOKEN_DOT){
                            c = c->next;
                            token = (token_t *) c->value;
                            continue;
                        }

                        if(token->key != TOKEN_COMMA && token->key != TOKEN_LBRACE){
                            parser_error(token, "bad definition of parent class!");
                        }

                        table_rpush(clscur->parents, (value_p)scp);
                    }
                }

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                clscur->start = array_rpush(code, CENT);

                /* current token must be '{' next it and then call statement */
                c = c->next;
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = statement(tls, base, c, clscur, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                // array_rpush(code, VAR);
                // array_rpush(code, value);
                // array_rpush(code, CHG);

                //array_rpush(code, CALL);
                clscur->end = array_rpush(code, CLEV);

                jmp->value = (value_t)array_rpush(code, NUL);
                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            else
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = class_name;
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }

                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);
                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;
                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            else
            if(token->key == TOKEN_LBRACKET){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = class_name;
                fun->type = FN_BRACKET;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACKET){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }

                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;
                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }

            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }

            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_THIS){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = clspar->key;
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_SUPER){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = clspar->super->key;
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_COUNT){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "count";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_EQ){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "=";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_EQEQ){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "==";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_RBRACE){
                c = c->next;
                token = (token_t *) c->value;
                }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_NEQ){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "!=";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_CARET){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "^";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_STAR){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "*";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;
                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_SLASH){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "/";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_RBRACE){
                c = c->next;
                token = (token_t *) c->value;
                }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_BACKSLASH){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "\\";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_PERCENT){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "%";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
            return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_PLUS){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "+";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_MINUS){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "-";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_GTEQ){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = ">=";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_LTEQ){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "<=";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_AND){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "&&";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_OR){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "||";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;
            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_LT){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "<";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;
                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }
                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_GT){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = ">";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;
                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_LTLT){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "<<";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;
                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_GTGT){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = ">>";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_LAND){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "&";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;
            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;
                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_LOR){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "|";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_MINUSGT){
            c = c->next;
            token = (token_t *) c->value;
            if(token->key == TOKEN_LPAREN){
                function_t *fun = (function_t *)malloc(sizeof(function_t));
                fun->key = "->";
                fun->type = FN_PAREN;
                fun->n = 0;
                fun->variables = table_create();
                fun->super = clspar;

                table_rpush(clspar->functions, (value_p)fun);

                array_rpush(code, JMP);
                iarray_t *jmp = array_rpush(code, 0);

                fun->start = array_rpush(code, ENT);

                c = c->next;
                token = (token_t *) c->value;
                while(token->key != TOKEN_RPAREN){
                    int ref = 0;
                    if(token->key == TOKEN_COMMA){
                        c = c->next;
                        token = (token_t *) c->value;
                        continue;
                    }
                    if(token->key == TOKEN_REF){
                        ref = 1;
                        c = c->next;
                        token = (token_t *) c->value;
                    }

                    if(token->key != TOKEN_ID){
                        parser_error(token, "bad parameters definition!");
                    }

                    fun->n++;
                    array_rpush(code, VAR);
                    array_rpush(code, (value_t)token->value);
                    if(ref > 0){
                        array_rpush(code, REF);
                    }
                    array_rpush(code, LD);

                    c = c->next;
                    token = (token_t *) c->value;
                }

                c = c->next;
                c = c->next;

                // body function
                token = (token_t *) c->value;

                while(token->key != TOKEN_RBRACE){
                    c = expression(tls, c, code);
                    token = (token_t *) c->value;

                    if(token->key != TOKEN_RBRACE){
                        c = c->next;
                        token = (token_t *) c->value;
                    }
                }

                fun->end = array_rpush(code, LEV);

                jmp->value = (value_t)array_rpush(code, NUL);

                if(c == tls->end){
                    return c;
                }
                c = c->next;
                continue;
            }
            c = c->previous;
            token = (token_t *) c->value;

            while(token->key != TOKEN_SEMICOLON){
                c = expression(tls, c, code);
                token = (token_t *) c->value;

                if(token->key != TOKEN_SEMICOLON){
                    c = c->next;
                    token = (token_t *) c->value;
                }
            }
            if(c == tls->end){
                return c;
            }
            c = c->next;
            continue;
        } else if(token->key == TOKEN_LBRACE){
            c = c->next;
            /*
            current token must be '{' then next token and
            call expression then state must be false
            */
            c = expression(tls, c, code);

            /* return token '}' */

            token = (token_t *) c->value;
            if(token->key != TOKEN_SEMICOLON){
                if(token->key != TOKEN_RBRACE){
                    parser_error(token, "must be close brace!");
                }
                c = c->next;
            }
            return c;
        } else if(token->key == TOKEN_DOT) {
            c = c->next;
            if(c == tls->end){
                return c;
            }
            continue;
        } else if(token->key == TOKEN_RBRACE) {
            return c;
        } else if(token->key == TOKEN_IMPORT){
            c = c->next;
            token = (token_t *) c->value;

            if(token->key != TOKEN_LPAREN){
                parser_error(token, "import, using : import('library-name or path', ...);!");
            }

            c = c->next;
            token = (token_t *) c->value;

            array_t *code_bytes = array_create();

            while(token->key != TOKEN_RPAREN){
                if(token->key == TOKEN_COMMA){
                    c = c->next;
                    token = (token_t *) c->value;
                    continue;
                }

                if(token->key != TOKEN_DATA){
                    parser_error(token, "import, library-path!");
                }

                if(strncmp((char *)token->value, "io", 2) == 0){
                    library_io(clspar, code_bytes);

                    c = c->next;
                    token = (token_t *) c->value;
                    continue;
                }
                else
                if(strncmp((char *)token->value, "file", 4) == 0){
                    library_file(clspar, code_bytes);

                    c = c->next;
                    token = (token_t *) c->value;
                    continue;
                }
                else
                if(strncmp((char *)token->value, "keyboard", 8) == 0){
                    library_keyboard(clspar, code_bytes);

                    c = c->next;
                    token = (token_t *) c->value;
                    continue;
                }
                else
                if(strncmp((char *)token->value, "time", 4) == 0){
                    library_time(clspar, code_bytes);

                    c = c->next;
                    token = (token_t *) c->value;
                    continue;
                }
                else {
                    FILE *fd;
                    char destination [ MAX_PATH ];

                    char *str_value = (char *)token->value;

                    if(*str_value != '/'){
                        char cwd[MAX_PATH];
                        if (getcwd(cwd, sizeof(cwd)) == NULL) {
                            perror("getcwd() error");
                            exit(-1);
                        }

                        utils_combine ( destination, cwd, str_value );
                    } else {
                        strcpy(destination, str_value);
                    }

                    if (!(fd = fopen(destination, "rb"))) {
                        if (!(fd = fopen(str_value, "rb"))) {
                            printf("could not open(%s)\n", str_value);
                            exit(-1);
                        }
                    }

                    // Current position
                    long64_t pos = ftell(fd);
                    // Go to end
                    fseek(fd, 0, SEEK_END);
                    // read the position which is the size
                    long64_t chunk = ftell(fd);
                    // restore original position
                    fseek(fd, pos, SEEK_SET);

                    char *buf;
                    if (!(buf = malloc(chunk + 1))) {
                        printf("could not malloc(%lld) for buf area\n", chunk);
                        exit(-1);
                    }

                    long64_t i;
                    // read the source file
                    if ((i = fread(buf, 1, chunk, fd)) < chunk) {
                        printf("read returned %lld\n", i);
                        exit(-1);
                    }

                    buf[i] = '\0';

                    fclose(fd);

                    table_t *tbl = table_create();
                    lexer(tbl, buf);

                    itable_t *c2 = tbl->begin;
                    while( c2 && c2 != tbl->end ){
                        c2 = statement(tbl, base, c2, clspar, code_bytes);
                        if(c2 != tls->end){
                            c2 = c2->next;
                        }
                    }

                    c = c->next;
                    token = (token_t *) c->value;
                    continue;
                }

                parser_error(token, "import, bad definition!");
            }

            iarray_t *c2 = code->end->previous;

            c2->next->previous = code_bytes->end->previous;
            code_bytes->end->previous->next = c2->next;

            c2->next = code_bytes->begin;
            code_bytes->begin->previous = c2;

            c = c->next;
            token = (token_t *) c->value;

            if(token->key == TOKEN_SEMICOLON){
                c = c->next;
                token = (token_t *) c->value;
            }

            continue;
        }


        break;
    }

    if((c != tls->end)){
        token_t *token = (token_t *) c->value;
        parser_error(token, "bad statement!");
    }

    return c;
}

class_t *
parser(table_t *tls, long64_t argc, char **argv, array_t *code)
{

    static char *global_name = "main";

    itable_t *c;

    class_t *clscur = (class_t *)malloc(sizeof(class_t));
    clscur->key = global_name;
    clscur->type = CLASS_BURN;

    clscur->parents = table_create();
    clscur->childrens = table_create();
    clscur->variables = table_create();
    clscur->functions = table_create();

    clscur->super = clscur;

    class_t *base = clscur;

    array_rpush(code, JMP);
    iarray_t *jmp = array_rpush(code, 0);

    clscur->start = array_rpush(code, CENT);


    c = tls->begin;
    while( c && c != tls->end ){
        c = statement(tls, base, c, clscur, code);
        if(c != tls->end){
            c = c->next;
        }
    }

    long64_t cnt = argc;
    while(argc > 0){
        char *data = (char *)*argv;

        if(valid_digit(*data)){
            if(check_hexadecimal(data)){
                array_rpush(code, IMM);
                array_rpush(code, (value_t)data);
                array_rpush(code, TP_NUMBER);
                array_rpush(code, PUSH);

                argc--;
                argv++;
                continue;
            }
            else if(check_integer(data)){
                array_rpush(code, IMM);
                array_rpush(code, (value_t)data);
                array_rpush(code, TP_NUMBER);
                array_rpush(code, PUSH);

                argc--;
                argv++;
                continue;
            }
            else if(check_double(data)){
                array_rpush(code, IMM);
                array_rpush(code, (value_t)data);
                array_rpush(code, TP_NUMBER);
                array_rpush(code, PUSH);

                argc--;
                argv++;
                continue;
            }
            else {
                array_rpush(code, IMM);
                array_rpush(code, (value_t)data);
                array_rpush(code, TP_NUMBER);
                array_rpush(code, PUSH);

                argc--;
                argv++;
                continue;
            }
        }
        else if(valid_alpha(*data) || white_space(*data)){
            array_rpush(code, IMM);
            array_rpush(code, (value_t)data);
            array_rpush(code, TP_DATA);
            array_rpush(code, PUSH);

            argc--;
            argv++;
            continue;
        }
        else {
            printf("bad argument!\n");
            exit(-1);
        }

        argc--;
        argv++;
    }

    array_rpush(code, IMM);
    array_rpush(code, cnt);
    array_rpush(code, TP_IMM);
    array_rpush(code, PUSH);

    array_rpush(code, VAR);
    array_rpush(code, (long64_t)global_name);
    array_rpush(code, SVPA);
    array_rpush(code, CALL);
    clscur->end = array_rpush(code, EXIT);

    jmp->value = (value_t)array_rpush(code, NUL);
    array_rpush(code, SETUP);
    array_rpush(code, (long64_t)global_name);

    return base;
}
