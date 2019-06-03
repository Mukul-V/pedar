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

static char *STR_NULL = "NULL";
static char *STR_CHAR = "CHAR";
static char *STR_NUMBER = "NUMBER";
static char *STR_DATA = "DATA";

/*
static char *STR_CLASS = "CLASS";
static char *STR_OCTAL = "OCTAL";
static char *STR_HEXADECIMAL = "HEXADECIMAL";
static char *STR_DOUBLE = "DOUBLE";
*/

#define FREE(ptr) \
	while(ptr){ \
		free((ptr)); \
		(ptr) = nullptr;\
	}

#define REALLOC(ptr, n, type) realloc(ptr, sizeof(type) * n)
#define CALLOC(n, type) calloc(n, sizeof(type))
#define MALLOC(n) malloc(n)

#define OBJECT_MALLOC(type) malloc(sizeof(object_type_t) + sizeof(object_level_t) + sizeof(type))
#define OBJECT_REALLOC(ptr, type) realloc(ptr, sizeof(object_type_t) + sizeof(object_level_t) + sizeof(type))

/* variable type */
typedef struct variable {
	string_t key;
	object_t *value;
} variable_t;

//OR LOR XOR AND LAND EQ NE LT LE GT GE LSHIFT RSHIFT ADD SUB MUL DIV MOD


function_t *
function_get(class_t *clspar, char *key, long64_t nparams, fn_type_t type);

function_t *
function_clone(class_t *clspar, function_t *funpar);

void
function_delete(function_t *fnpar);



class_t *
class_clone(class_t *clspar);

void
class_delete(class_t *clspar);

/* search in parents for find a class by key */
class_t *
class_get(class_t *clspar, char *key);

/* search in childrens for find a class by key */
class_t *
class_find(class_t *clspar, char *key);

long64_t
class_sizeof(class_t *clspar);

variable_t *
class_getvar(class_t *clspar, char *key);



variable_t *
variable_get(table_t *variables, char *key);

variable_t *
variable_clone(variable_t *varpar);

void
variable_clear(variable_t *varpar) ;

void
variable_delete(variable_t *varpar);



long64_t
data_sizeof(table_t *tbl);

table_t *
data_clone(table_t *tbpar);

table_t *
data_from(char *str);

char *
data_to(table_t *tbl);

itable_t *
data_print(table_t *tbl, itable_t *c, itable_t *e);

void
data_delete(table_t *tbl);



long64_t
object_destroy(itable_t *it);

void
object_delete(object_t *obj);

long64_t
object_sizeof(object_t *obj);

long64_t
object_sort(long64_t *obj_1, long64_t *obj_2){
	object_t *obj1 = (object_t *)obj_1;
	object_t *obj2 = (object_t *)obj_2;
	if(obj1->type == TP_NUMBER && obj2->type == TP_NUMBER){
		return obj1->num < obj2->num;
	}
	return 0;
}

void
object_delete(object_t *obj)
{
	if(obj->type == TP_NULL){
		if(obj->level == LEVEL_TEMPRATORY){
			FREE(obj);
		}
        return;
    }
	else if(obj->type == TP_NUMBER){
		if(obj->level == LEVEL_TEMPRATORY){
			FREE(obj);
		}
        return;
    }
	else if(obj->type == TP_CHAR){
		if(obj->level == LEVEL_TEMPRATORY){
			FREE(obj);
		}
        return;
    }
    else if(obj->type == TP_DATA){
		if(obj->level == LEVEL_TEMPRATORY){
			//data_delete((table_t *)obj->ptr);
			//FREE(obj);
		}
        return;
    }
    else if(obj->type == TP_CLASS){
		if(obj->level == LEVEL_TEMPRATORY){
			//class_delete((class_t *)obj->ptr);
			//FREE(obj);
		}
        return;
    }
}

long64_t
object_destroy(itable_t *it)
{
	object_t *obj;
	if(!(obj = (object_t *)it->value)){
		printf("free frame, object not defined!\n");
		exit(-1);
	}
    //object_delete(obj);
    FREE(it);
    return 1;
}

long64_t
object_sizeof(object_t *obj)
{
    if(obj->type == TP_NULL){
        return sizeof(object_t);
    }
    else if(obj->type == TP_CHAR){
        return sizeof(char);
    }
    else if(obj->type == TP_NUMBER){
        return sizeof(double);
    }
    else if(obj->type == TP_DATA){
        return data_sizeof((table_t *)obj->ptr);
    }
    else if(obj->type == TP_CLASS){
        return class_sizeof((class_t *)obj->ptr);
    }

    printf("object sizeof, unknown type.\n");
    exit(-1);
}

object_t *
object_clone(object_t *obj)
{
    object_t *res = OBJECT_MALLOC(double);
    res->type = obj->type;
    if(obj->type == TP_NULL){
        res->ptr = 0;
        return res;
    }
    else if(obj->type == TP_CHAR){
        res->num = obj->num;
        return res;
    }
    else if(obj->type == TP_NUMBER){
        res->num = obj->num;
        return res;
    }
    else if(obj->type == TP_DATA){
        res->ptr = data_clone((table_t *)obj->ptr);
        return res;
    }
    else if(obj->type == TP_CLASS){
        res->ptr = obj->ptr;
        return res;
    }

    printf("object clone, unknown type.\n");
    exit(-1);
}


/* get function from current class and parrent classes */
function_t *
function_get(class_t *clspar, char *key, long64_t nparams, fn_type_t type)
{
    function_t *fn;
    itable_t *c;
    for(c = clspar->functions->begin; c != clspar->functions->end; c = c->next){
        fn = (function_t *)c->value;
        if((strncmp(key, fn->key, max(strlen(key), strlen(fn->key))) == 0) && (fn->n == nparams) && (fn->type == type)){
            return fn;
        }
    }

    for(c = clspar->parents->begin; c && (c != clspar->parents->end); c = c->next){
        class_t *clscur = (class_t *)c->value;
        if((fn = function_get(clscur, key, nparams, type))){
            return fn;
        }
    }

    if(clspar->super && clspar->super != clspar){
        if((fn = function_get(clspar->super, key, nparams, type))){
            return fn;
        }
    }

    return nullptr;
}

function_t *
function_clone(class_t *clspar, function_t *funpar)
{
    function_t *fun = (function_t *)malloc(sizeof(function_t));
    fun->key = funpar->key;
    fun->type = funpar->type;
    fun->n = funpar->n;

    fun->variables = table_create();

    fun->start = funpar->start;
    fun->end = funpar->end;

    fun->super = clspar;

    return fun;
}

void
function_delete(function_t *fnpar)
{
    itable_t *b, *n;
    for(b = fnpar->variables->begin; b != fnpar->variables->end; b = n){
        n = b->next;
        variable_t *var;
        if(!(var = (variable_t *)b->value)){
            continue;
        }
        variable_delete(var);
        //free(b);
    }
    FREE(fnpar);
}


class_t *
class_clone(class_t *clspar)
{
    class_t *clscur = (class_t *)malloc(sizeof(class_t));
    clscur->key = clspar->key;
    clscur->type = CLASS_RAW;

    clscur->parents = table_create();
    clscur->childrens = table_create();
    clscur->functions = table_create();
    clscur->variables = table_create();

    clscur->start = clspar->start;
    clscur->end = clspar->end;

    clscur->super = clspar->super;

    itable_t *c;
    class_t *cls;
    for(c = clspar->parents->begin; c != clspar->parents->end; c = c->next){
        if(!(cls = (class_t *)c->value)){
            printf("clone: class is null!\n");
            exit(-1);
        }
        table_rpush(clscur->parents, (value_p)cls);
    }

    for(c = clspar->childrens->begin; c != clspar->childrens->end; c = c->next){
        if(!(cls = (class_t *)c->value)){
            printf("clone: class is null!\n");
            exit(-1);
        }
        table_rpush(clscur->childrens, (value_p)cls);
    }

    function_t *fn;
    for(c = clspar->functions->begin; c != clspar->functions->end; c = c->next){
        if(!(fn = (function_t *)c->value)){
            printf("clone: function not clone!\n");
            exit(-1);
        }
        table_rpush(clscur->functions, (value_p)fn);
    }

    return clscur;
}

void
class_delete(class_t *clspar)
{
    itable_t *b, *n;
    for(b = clspar->variables->begin; b != clspar->variables->end; b = n){
        n = b->next;
        variable_t *var;
        if(!(var = (variable_t *)b->value)){
            continue;
        }
        if(var->value->type != TP_CLASS){
            variable_delete(var);
        }
    }

    if(clspar && clspar->super && (clspar != clspar->super)){
        class_delete(clspar->super);
    }

    FREE(clspar);
}

/* search in parents for find a class by key */
class_t *
class_get(class_t *clspar, char *key)
{
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
		if(!(clsres = class_get(cls, key))){
			continue;
		}
	  	return clsres;
	}

	if(strncmp(key, clspar->key, max(strlen(key), strlen(clspar->key))) == 0){
		return clspar;
	}

	return nullptr;
}

table_t *
class_get_name(class_t *clspar)
{
	table_t *tbl = table_create();
	object_t *obj;
	int j = 0;
	while(clspar && (clspar != clspar->super)){
		if(j++ > 0){
			obj = OBJECT_MALLOC(double);
			obj->type = TP_CHAR;
			obj->num = '.';
			table_lpush(tbl, (value_p)obj);
		}
		long64_t i = 0;
		for(i = strlen(clspar->key); i > 0; i--){
			obj = OBJECT_MALLOC(double);
			obj->type = TP_CHAR;
			obj->num = clspar->key[i - 1];
			table_lpush(tbl, (value_p)obj);
		}
		clspar = clspar->super;
	}
	return tbl;
}

table_t *
class_get_path(class_t *clspar)
{
	table_t *tbl = table_create();
	while(clspar && (clspar != clspar->super)){
		table_rpush(tbl, (value_p)clspar);
		clspar = clspar->super;
	}
	return tbl;
}

/* search in childrens for find a class by key */
class_t *
class_find(class_t *clspar, char *key)
{
	itable_t *c; class_t *clsres;

	for(c = clspar->childrens->begin; c && (c != clspar->childrens->end); c = c->next){
		class_t *cls = (class_t *)c->value;
		if(!((char*)cls->key)){
			continue;
		}

		if(strncmp(key, (char *)cls->key, max(strlen(key), strlen(cls->key))) == 0){
			return cls;
		}

		if(!(clsres = class_find(cls, key))){
			continue;
		}

		return clsres;
	}

	return nullptr;
}

long64_t
class_sizeof(class_t *clspar)
{
	long64_t res = 0;
	variable_t *var;
	itable_t *p;
	for(p = clspar->variables->begin; p != clspar->variables->end; p = p->next){
		var = (variable_t *)p->value;
		res += object_sizeof(var->value);
	}

	for(p = clspar->childrens->begin; p != clspar->childrens->end; p = p->next){
    	res += class_sizeof((class_t *)p->value);
	}

	return res;
}

variable_t *
class_getvar(class_t *clspar, char *key)
{
	variable_t *var;
	itable_t *c;
	for(c = clspar->variables->begin; c != clspar->variables->end; c = c->next){
		var = (variable_t *)c->value;
		if(strncmp(key, var->key, max(strlen(key), strlen(var->key))) == 0){
			return var;
		}
	}

	if(clspar->super != clspar){
		if((var = class_getvar(clspar->super, key))){
			return var;
		}
	}

	for(c = clspar->parents->begin; c != clspar->parents->end; c = c->next){
	  	class_t *clscur = (class_t *)c->value;
		if((var = class_getvar(clscur, key))){
			return var;
		}
	}

	return nullptr;
}


variable_t *
variable_get(table_t *variables, char *key)
{
	itable_t *c;
	for(c = variables->begin; c != variables->end; c = c->next){
		variable_t *var = (variable_t *)c->value;
		if(strncmp(key, var->key, max(strlen(key), strlen(var->key))) == 0){
			return var;
		}
	}
	return nullptr;
}


variable_t *
variable_clone(variable_t *varpar)
{
    variable_t *var;
    if(!(var = MALLOC(sizeof(variable_t)))){
        printf("variable clone, variable not defined.\n");
        exit(-1);
    }
    var->key = varpar->key;
    var->value = object_clone(varpar->value);
    return var;
}

void
variable_delete(variable_t *var)
{
    object_delete(var->value);
    FREE(var);
}



long64_t
data_sizeof(table_t *tbl)
{
    long64_t res = 0;
    itable_t *t;
    for(t = tbl->begin; t != tbl->end; t = t->next){
        res += object_sizeof((object_t *) t->value);
    }
    return res;
}

table_t *
data_clone(table_t *tbpar)
{
    table_t *tbl = table_create();
    itable_t *t;
    for(t = tbpar->begin; t != tbpar->end; t = t->next){
        object_t *obj;
        if(!(obj = object_clone((object_t *)t->value))){
            printf("data clone, object not clone!\n");
            exit(-1);
        }
        table_rpush(tbl, (value_p)obj);
    }
    return tbl;
}

table_t *
data_from(char *str)
{
    table_t *tbl = table_create();
    long64_t i;
    for(i = 0; i < strlen(str); i++){
        object_t *obj;
        if(!(obj = OBJECT_MALLOC(char))){
            printf("string to data convert, bad memory!\n");
            exit(-1);
        }
        obj->type = TP_CHAR;
        obj->num = str[i];
        table_rpush(tbl, (value_p)obj);
    }
    return tbl;
}

char *
data_to(table_t *tbl)
{
    char *str = malloc(table_count(tbl) * sizeof(char));
    long64_t i = 0;
    itable_t *b;
    for(b = tbl->begin; b && (b != tbl->end); b = b->next){
        object_t *obj = (object_t *)b->value;
        if(obj->type == TP_CHAR){
            str[i++] = (char)obj->num;
            continue;
        }
    }
    return str;
}

table_t *
data_format(table_t *tbl, table_t *format, table_t *tbres)
{
	itable_t *t = tbl->begin, *f = format->begin;
	object_t *esp, *obj;

    while( t != tbl->end ){
        obj = (object_t *)t->value;

        if(obj->type == TP_NULL){
			long64_t i;
			for(i = 0; i < strlen(STR_NULL); i++){
				if(!(esp = OBJECT_MALLOC(char))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                esp->type = TP_CHAR;
                esp->num = (char)STR_NULL[i];
				table_rpush(tbres, (value_p)esp);
			}
			if(!(esp = OBJECT_MALLOC(char))){
				printf("object, bad memory!\n");
				exit(-1);
			}
			esp->type = TP_CHAR;
			esp->num = ' ';
			table_rpush(tbres, (value_p)esp);
            t = t->next;
            continue;
        }
		else
		if(obj->type == TP_CHAR){
			long64_t num = 0;
			if(obj->num == '%'){
				t = t->next;

				obj = (object_t *)t->value;

				if(obj->num == 's'){
					obj = (object_t *)f->value;
					f = f->next;
					if(obj->type != TP_DATA){
						printf("%%s must be input string data!\n");
						exit(-1);
					}
					data_format((table_t *)obj->ptr, format, tbres);
					t = t->next;
					continue;
				}
				else
				if(obj->num == '.'){
					t = t->next;
					obj = (object_t *)t->value;
					if(obj->type != TP_CHAR){
						printf("object not a char type!\n");
						exit(-1);
					}
					while(valid_digit(obj->num)){
						if(obj->type != TP_CHAR){
							printf("object not a char type!\n");
							exit(-1);
						}
						num = num * 10 + (char)obj->num - '0';
						t = t->next;
						obj = (object_t *)t->value;
					}
				}

				if(obj->num == 'n'){
					obj = (object_t *)f->value;
					f = f->next;

					char *fmt = MALLOC(sizeof(char) * 255);
					sprintf(fmt, "%%.%lldf", num);

					char *str_num = MALLOC(sizeof(char) * 255);
					if(((obj->num - (long64_t)obj->num) != 0) || (num > 0)){
						sprintf(str_num, fmt, obj->num);
					}
					else {
						sprintf(str_num, "%lld", (long64_t)obj->num);
					}

					FREE(fmt);

					long64_t i;
					for(i = 0; i < strlen(str_num); i++){
						if(!(esp = OBJECT_MALLOC(char))){
							printf("object, bad memory!\n");
							exit(-1);
						}
						esp->type = TP_CHAR;
						esp->num = (char)str_num[i];
						table_rpush(tbres, (value_p)esp);
					}

					FREE(str_num);

					t = t->next;
					continue;
				}

				if(!(esp = OBJECT_MALLOC(char))){
					printf("object, bad memory!\n");
					exit(-1);
				}
				esp->type = TP_CHAR;
				esp->num = obj->num;
				table_rpush(tbres, (value_p)esp);

				t = t->next;
				continue;
			}

			if(!(esp = OBJECT_MALLOC(char))){
				printf("object, bad memory!\n");
				exit(-1);
			}
			esp->type = TP_CHAR;
			esp->num = obj->num;
			table_rpush(tbres, (value_p)esp);
			t = t->next;
			continue;
		}
		else
		if(obj->type == TP_CLASS){
			class_t *cls = (class_t *)obj->ptr;
			long64_t i;
			for(i = 0; i < strlen(cls->key); i++){
				if(!(esp = OBJECT_MALLOC(char))){
					printf("object, bad memory!\n");
					exit(-1);
				}
				esp->type = TP_CHAR;
				esp->num = (char)cls->key[i];
				table_rpush(tbres, (value_p)esp);
			}
			if(!(esp = OBJECT_MALLOC(char))){
				printf("object, bad memory!\n");
				exit(-1);
			}
			esp->type = TP_CHAR;
			esp->num = ' ';
			table_rpush(tbres, (value_p)esp);
			t = t->next;
			continue;
		}
		else
		if(obj->type == TP_NUMBER){
			char *str_num = MALLOC(sizeof(char) * 255);
			if((obj->num - (long64_t)obj->num) != 0){
				sprintf(str_num, "%.16f", obj->num);
			}
			else {
				sprintf(str_num, "%lld", (long64_t)obj->num);
			}
			long64_t i;
			for(i = 0; i < strlen(str_num); i++){
				if(!(esp = OBJECT_MALLOC(char))){
					printf("object, bad memory!\n");
					exit(-1);
				}
				esp->type = TP_CHAR;
				esp->num = (char)str_num[i];
				table_rpush(tbres, (value_p)esp);
			}

			FREE(str_num);

			if(!(esp = OBJECT_MALLOC(char))){
				printf("object, bad memory!\n");
				exit(-1);
			}
			esp->type = TP_CHAR;
			esp->num = ' ';
			table_rpush(tbres, (value_p)esp);
			t = t->next;
			continue;
		}
		else
		if(obj->type == TP_DATA){
			data_format((table_t *)obj->ptr, format, tbres);
			t = t->next;
			continue;
		}

		printf("this type not a acceptable!\n");
		exit(-1);
	}

	return tbres;
}

void
data_delete(table_t *tbl)
{
    itable_t *b, *n;
    for( b = tbl->begin; b != tbl->end; b = n ){
        n = b->next;
        object_delete((object_t *)b->value);
        FREE(b);
    }
    FREE(tbl);
}

long64_t
data_compare(table_t *tbl1, table_t *tbl2)
{
	if(table_count(tbl1) != table_count(tbl2)){
		return 0;
	}

	itable_t *t1 = tbl1->begin, *t2 = tbl2->begin;
	object_t *obj1, *obj2;

	while( t1 != tbl1->end && t2 != tbl2->end){
		obj1 = (object_t *)t1->value;
		obj2 = (object_t *)t2->value;
		if(obj1->type != obj2->type){
			return 0;
		}
		if(obj1->type == TP_NULL){
			if(obj1->ptr != obj2->ptr){
				return 0;
			}
		}
		else
		if(obj1->type == TP_NUMBER){
			if(obj1->num != obj2->num){
				return 0;
			}
		}
		else
		if(obj1->type == TP_CHAR){
			if(obj1->num != obj2->num){
				return 0;
			}
		}
		else
		if(obj1->type == TP_CLASS){
			table_t *tn1 = class_get_name((class_t *)obj1->ptr);
			table_t *tn2 = class_get_name((class_t *)obj2->ptr);
			long64_t r = data_compare(tn1,tn2);
			data_delete(tn1);
			data_delete(tn2);
			return r;
		}

		t1 = t1->next;
		t2 = t2->next;
	}

	return 1;
}

void
eval(class_t *base, array_t *code)
{
    long64_t op;

    variable_t *egx = nullptr;

    /* ecx save refrence of current class */
    class_t *ecx = nullptr;

    object_t *eax = nullptr;
    object_t *esp = nullptr;

	table_t *stack_ebx = table_create();
	table_t *stack_ecx = table_create();
	table_t *stack_edx = table_create();

    /* epx save refrence of parrent class */
    class_t *epx = nullptr;
    table_t *stack_epx = table_create();

    /* efx save refrence of path func in variables */
    function_t *efx = nullptr;
    table_t *stack_efx = table_create();

    table_t *frame = nullptr;
    table_t *stack_frame = table_create();

    iarray_t *c = code->begin;

    itable_t *rp;

    while( c != code->end ){
        // get next operation code
        op = c->value;

        //printf("%lld\n", c->value);

        if (op == NUL) {
        	c = c->next;
        	continue;
        }
        else if (op == LOPB){
            c = c->next;
            continue;
        }
        else if (op == LOPE){
            c = c->next;
            continue;
        }

        //OR LOR XOR AND LAND EQ NE LT LE GT GE LSHIFT RSHIFT ADD SUB MUL DIV MOD RAR
        else if (op == OR){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
                printf("[or], you can use of '||' between two object!\n");
                exit(-1);
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[or] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = (long64_t)esp->num || (long64_t)eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , "||",1, FN_PAREN))){
                    printf("eval: operator '||' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[or] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , "||",1, FN_PAREN))){
                    printf("eval: operator '||' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[or] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("[or] operator not defined!\n");
            exit(-1);
        }
        else if (op == LOR){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
                printf("[LOGIC OR], you can use of '|' between two integer number!\n");
                exit(-1);
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[LOGIC OR] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = (long64_t)esp->num | (long64_t)eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , "|",1, FN_PAREN))){
                    printf("eval: operator '|' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[LOGIC OR] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , "|",1, FN_PAREN))){
                    printf("eval: operator '|' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[LOGIC OR] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("[LOGIC OR] operator not defined!\n");
            exit(-1);
        }
        else if (op == XOR){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
                printf("[^], you can use of '^' between two object!\n");
                exit(-1);
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[^] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = (long64_t)esp->num ^ (long64_t)eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , "^",1, FN_PAREN))){
                    printf("eval: operator '^' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[XOR] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , "^",1, FN_PAREN))){
                    printf("eval: operator '^' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[XOR] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("[XOR] operator not defined!\n");
            exit(-1);
        }
        else if (op == AND){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
                printf("[&&], you can use of '&&' between two object!\n");
                exit(-1);
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[&&] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = (long64_t)esp->num && (long64_t)eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , "&&",1, FN_PAREN))){
                    printf("eval: operator '&&' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[&&] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , "&&",1, FN_PAREN))){
                    printf("eval: operator '&&' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[&&] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("[&&] operator not defined!\n");
            exit(-1);
        }
        else if (op == LAND){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
                printf("[&], you can use of '&' between two object!\n");
                exit(-1);
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[&] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = (long64_t)esp->num & (long64_t)eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , "&",1, FN_PAREN))){
                    printf("eval: operator '&' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[&] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , "&",1, FN_PAREN))){
                    printf("eval: operator '&' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[&] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("[&] operator not defined!\n");
            exit(-1);
        }
        else if (op == EQ){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
                printf("[==], you can use of '==' between two object!\n");
                exit(-1);
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[==] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = esp->num == eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , "==",1, FN_PAREN))){
                    printf("eval: operator '==' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[==] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , "==",1, FN_PAREN))){
                    printf("eval: operator '==' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[==] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
			else if(esp->type == TP_DATA && eax->type == TP_DATA){
				object_t *obj;
				if(!(obj = OBJECT_MALLOC(double))){
					printf("[==] object not defined!\n");
					exit(-1);
				}

				obj->type = TP_NUMBER;
				obj->num = data_compare((table_t *)esp->ptr, (table_t *)eax->ptr);

				object_delete(esp);
				object_delete(eax);

				eax = obj;
				c = c->next;
				continue;
			}

            printf("[==] operator not defined!\n");
            exit(-1);
        }
        else if (op == NE){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
                printf("[!=], you can use of '!=' between two object!\n");
                exit(-1);
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[!=] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = esp->num != eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , "!=",1, FN_PAREN))){
                    printf("eval: operator '!=' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[!=] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , "!=",1, FN_PAREN))){
                    printf("eval: operator '!=' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[!=] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
			else if(esp->type == TP_DATA && eax->type == TP_DATA){
				object_t *obj;
				if(!(obj = OBJECT_MALLOC(double))){
					printf("[==] object not defined!\n");
					exit(-1);
				}

				obj->type = TP_NUMBER;
				obj->num = !data_compare((table_t *)esp->ptr, (table_t *)eax->ptr);

				object_delete(esp);
				object_delete(eax);

				eax = obj;
				c = c->next;
				continue;
			}
            printf("[!=] operator not defined!\n");
            exit(-1);
        }
        else if (op == LT){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
                printf("[<], you can use of '<' between two object!\n");
                exit(-1);
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[<] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = esp->num < eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , "<",1, FN_PAREN))){
                    printf("eval: operator '<' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[<] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , "<",1, FN_PAREN))){
                    printf("eval: operator '<' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[<] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("[<] operator not defined!\n");
            exit(-1);
        }
        else if (op == LE){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
                printf("[<=], you can use of '<=' between two object!\n");
                exit(-1);
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[<=] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = esp->num <= eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , "<=",1, FN_PAREN))){
                    printf("eval: operator '<=' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[<=] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , "<=",1, FN_PAREN))){
                    printf("eval: operator '<=' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[<=] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("[<=] operator not defined!\n");
            exit(-1);
        }
        else if (op == GT){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
                printf("[>], you can use of '>' between two object!\n");
                exit(-1);
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[>] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = esp->num > eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , ">",1, FN_PAREN))){
                    printf("eval: operator '>' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[>] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , ">",1, FN_PAREN))){
                    printf("eval: operator '>' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[>] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("[>] operator not defined!\n");
            exit(-1);
        }
        else if (op == GE){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
                printf("[>=], you can use of '>=' between two object!\n");
                exit(-1);
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[>=] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = esp->num >= eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , ">=",1, FN_PAREN))){
                    printf("eval: operator '>=' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[>=] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , ">=",1, FN_PAREN))){
                    printf("eval: operator '>=' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[>=] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("[>=] operator not defined!\n");
            exit(-1);
        }
        else if (op == LSHIFT){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
                printf("[<<], you can use of '<<' between two object!\n");
                exit(-1);
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[<<] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = (long64_t)esp->num << (long64_t)eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , "<<",1, FN_PAREN))){
                    printf("eval: operator '<<' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[<<] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , "<<",1, FN_PAREN))){
                    printf("eval: operator '<<' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[<<] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("[<<] operator not defined!\n");
            exit(-1);
        }
        else if (op == RSHIFT){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
                printf("[>>], you can use of '>>' between two object!\n");
                exit(-1);
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[>>] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = (long64_t)esp->num >> (long64_t)eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , ">>",1, FN_PAREN))){
                    printf("eval: operator '>>' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[>>] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , ">>",1, FN_PAREN))){
                    printf("eval: operator '>>' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[>>] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("[>>] operator not defined!\n");
            exit(-1);
        }
        else if (op == ADD){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
				object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[+] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 0;

                esp = obj;
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[+] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = esp->num + eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , "+",1, FN_PAREN))){
                    printf("eval: operator '+' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[+] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , "+",1, FN_PAREN))){
                    printf("eval: operator '+' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[+] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("[+] operator not defined!\n");
            exit(-1);
        }
        else if (op == SUB){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
				object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[-] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 0;

                esp = obj;
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[-] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = esp->num - eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , "-",1, FN_PAREN))){
                    printf("eval: operator '-' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[-] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , "-",1, FN_PAREN))){
                    printf("eval: operator '-' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[-] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("[-] operator not defined!\n");
            exit(-1);
        }
        else if (op == MUL){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
                printf("[*], you can use of '*' between two object!\n");
                exit(-1);
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[*] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = esp->num * eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , "*",1, FN_PAREN))){
                    printf("eval: operator '*' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[*] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , "*",1, FN_PAREN))){
                    printf("eval: operator '*' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[*] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("[*] operator not defined!\n");
            exit(-1);
        }
        else if (op == DIV){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
                printf("[/], you can use of '/' between two object!\n");
                exit(-1);
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[/] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = esp->num / eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , "/",1, FN_PAREN))){
                    printf("eval: operator '/' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[/] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , "/",1, FN_PAREN))){
                    printf("eval: operator '/' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[/] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("[/] operator not defined!\n");
            exit(-1);
        }
        else if (op == MOD){
            rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
                printf("[%%], you can use of '%%' between two object!\n");
                exit(-1);
            }

            if(esp->type == TP_NUMBER && eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[%%] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = (long64_t)esp->num % (long64_t)eax->num;

                object_delete(esp);
                object_delete(eax);

                eax = obj;
                c = c->next;
                continue;
            }
            else if(esp->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)esp->ptr , "%",1, FN_PAREN))){
                    printf("eval: operator '%%' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[%%] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , "%",1, FN_PAREN))){
                    printf("eval: operator '%%' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)esp);

                object_t *obj;

                if(!(obj = OBJECT_MALLOC(double))){
                    printf("[%%] object not defined!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("[%%] operator not defined!\n");
            exit(-1);
        }
        else if (op == RAR){
			if(eax == nullptr){
				printf("[->], you can use of '->' after a object!\n");
                exit(-1);
			}

            table_rpush(stack_efx, (value_p)efx);

            if(!(efx = function_get((class_t *)eax->ptr , "->",1, FN_PAREN))){
                printf("eval: operator '->' not define in this function!\n");
                exit(-1);
            }

            table_t *newframe = table_create();
            table_rpush(newframe, (value_p)eax);

            object_t *obj;

            if(!(obj = OBJECT_MALLOC(double))){
                printf("[->] object not defined!\n");
                exit(-1);
            }

            obj->type = TP_NUMBER;
            obj->num = 1;

            table_rpush(newframe, (value_p)obj);

            table_rpush(frame, (value_p)c->next);
            table_rpush(stack_frame, (value_p)frame);

            frame = newframe;

            c = (iarray_t *)efx->start;
            continue;
        }


        else if (op == VAR){
            c = c->next;
            if(ecx){
				class_t *rbx = nullptr;
				if((rbx = class_find(ecx, (char *)c->value))){
					if(!(egx = MALLOC(sizeof(variable_t)))){
		                printf("object, bad memory!\n");
		                exit(-1);
		            }
					egx->key = (char *)c->value;
					if(!(egx->value = OBJECT_MALLOC(void *))){
				        printf("object, bad memory!\n");
				        exit(-1);
				    }
					egx->value->type = TP_CLASS;
					egx->value->level = LEVEL_REGISTER;
					egx->value->ptr = rbx;
					table_rpush(efx ? efx->variables : epx->variables, (value_p)egx);
					eax = egx->value;
					c = c->next;
					continue;
				}

				if((egx = class_getvar(ecx, (char *)c->value))){
                    eax = egx->value;
                    c = c->next;
                    continue;
                }
            }

			if(efx){
                if((egx = variable_get(efx->variables, (char *)c->value))){
                    eax = egx->value;
                    c = c->next;
                    continue;
                }
            }

            if((egx = class_getvar(epx, (char *)c->value))){
                eax = egx->value;
                c = c->next;
                continue;
            }

            if(!(egx = MALLOC(sizeof(variable_t)))){
                printf("object, bad memory!\n");
                exit(-1);
            }

			class_t *rbx = nullptr;
			if(!(rbx = class_get(ecx ? ecx : epx, (char *)c->value))){
				rbx = class_get(base, (char *)c->value);
			}

            egx->key = (char *)c->value;

			if(!(egx->value = OBJECT_MALLOC(void *))){
				printf("object, bad memory!\n");
				exit(-1);
			}
			egx->value->level = LEVEL_REGISTER;
			egx->value->type = rbx ? TP_CLASS : TP_NULL;
			egx->value->ptr = rbx;

            table_rpush(efx ? efx->variables : epx->variables, (value_p)egx);

            eax = egx->value;

            c = c->next;
            continue;
        }
        else if (op == IMM){
            // load immediate value to eax
            c = c->next;
            eax = nullptr;
            iarray_t *next = c->next;

            if(next->value == TP_NULL){
                if(!(eax = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                eax->type = TP_NULL;
                eax->ptr = 0;
                c = next->next;
                continue;
            }
            else if(next->value == TP_IMM){
                if(!(eax = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                eax->type = TP_NUMBER;
                eax->num = (long64_t)c->value;
                c = next->next;
                continue;
            }
            else if(next->value == TP_NUMBER){
                if(!(eax = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                eax->type = TP_NUMBER;
                eax->num = utils_atof((char *)c->value);
                c = next->next;
                continue;
            }
            else if(next->value == TP_CHAR){
                if(!(eax = OBJECT_MALLOC(char))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                eax->type = TP_CHAR;
                eax->num = (char)c->value;
                c = next->next;
                continue;
            }
            else if(next->value == TP_DATA){
                if(!(eax = OBJECT_MALLOC(void *))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                eax->type = TP_DATA;
                eax->ptr = data_from ((char *)c->value);
                c = next->next;
                continue;
            }

            printf("[IMM] unknown type %lld!\n", next->value);
            exit(-1);
        }
        else if (op == DATA){
            c = c->next;
            long64_t cnt = c->value;

            table_t *tbl = table_create();

            long64_t i;
            for(i = 0; i < cnt; i++){
                rp = table_rpop(frame);
                table_lpush(tbl, rp->value);
            }

			variable_t *ebx;
			rp = table_rpop(stack_ebx);
			ebx = (variable_t *)rp->value;

            if(ebx && ebx->value->ptr){
                // call subroutine
                if(ebx->value->type == TP_DATA){
                    rp = table_rpop(tbl);
                    esp = (object_t *)rp->value;

                    if(!(rp = table_at((table_t *)ebx->value->ptr, esp->num))){
                        printf("[key] is gritter than array size!\n");
                        exit(-1);
                    }

                    if(!(rp->value = rp->value ? rp->value : OBJECT_MALLOC(double))){
                        printf("object, bad memory!\n");
                        exit(-1);
                    }

                    eax = (object_t *)rp->value;

                    c = c->next;
                    continue;
                }
                else if(ebx->value->type == TP_CLASS){
                    class_t *clscur = (class_t *)ebx->value->ptr;

                    table_rpush(stack_efx, (value_p)efx);

                    if(!(efx = function_get(clscur ,clscur->key ,cnt, FN_BRACKET))){
                        printf("call %s[], not define this function!\n", (char *)clscur->key);
                        exit(-1);
                    }

                    table_rpush(frame, (value_p)c->next);
                    table_rpush(stack_frame, (value_p)frame);

                    frame = tbl;

                    c = (iarray_t *)efx->start;
                    continue;
                }

                printf("call %s[], not define for this variable!\n", ebx->key);
                exit(-1);
            }

            if(!(eax = OBJECT_MALLOC(void *))){
                printf("object, bad memory!\n");
                exit(-1);
            }

            eax->type = TP_DATA;
            eax->ptr = tbl;

            c = c->next;
            continue;
        }
        else if (op == SD){
            // save data to address, value in eax, address on stack_frame
			if(!(rp = table_rpop(frame))){
                printf("save data, bad pop data!\n");
                exit(-1);
            }

            if(!(esp = (object_t *)rp->value)){
				printf("save data, bad pop data!\n");
                exit(-1);
            }

            if(eax->type == TP_NULL){
				if(esp->type != eax->type){
					esp->ptr = OBJECT_REALLOC(esp->ptr, void *);
				}
                esp->ptr = eax->ptr;
            }
            else if(eax->type == TP_CHAR){
				if(esp->type != eax->type){
					esp->ptr = OBJECT_REALLOC(esp->ptr, char);
				}
                esp->num = eax->num;
            }
            else if(eax->type == TP_NUMBER){
				if(esp->type != eax->type){
					esp->ptr = OBJECT_REALLOC(esp->ptr, double);
				}
                esp->num = eax->num;
            }
            else if(eax->type == TP_DATA){
				if(esp->type != eax->type){
					esp->ptr = OBJECT_REALLOC(esp->ptr, void *);
				}
                esp->ptr = data_clone((table_t *)eax->ptr);
            }
            else if(eax->type == TP_CLASS){
				if(esp->type != eax->type){
					esp->ptr = OBJECT_REALLOC(esp->ptr, void *);
				}
                esp->ptr = (class_t *)eax->ptr;
            }

            esp->type = eax->type;

            c = c->next;
            continue;
        }
        else if (op == LD){
            rp = table_rpop(frame);

            if(!(esp = (object_t *)rp->value)){
                printf("load data, bad pop data!\n");
                exit(-1);
            }

			if(eax->level == LEVEL_REFRENCE){
				esp->level = LEVEL_REFRENCE;
				egx->value = esp;
				object_delete(eax);
				eax = esp;
				c = c->next;
	            continue;
			}

            if(esp->type == TP_NULL){
				if(esp->type != eax->type){
					eax->ptr = OBJECT_REALLOC(eax->ptr, void *);
				}
                eax->ptr = esp->ptr;
            }
            else if(esp->type == TP_CHAR){
				if(esp->type != eax->type){
					eax->ptr = OBJECT_REALLOC(eax->ptr, char);
				}
                eax->num = esp->num;
            }
            else if(esp->type == TP_NUMBER){
				if(esp->type != eax->type){
					eax->ptr = OBJECT_REALLOC(eax->ptr, double);
				}
                eax->num = esp->num;
            }
            else if(esp->type == TP_DATA){
				if(esp->type != eax->type){
					eax->ptr = OBJECT_REALLOC(eax->ptr, void *);
				}
                eax->ptr = esp->ptr;
            }
            else if(esp->type == TP_CLASS){
				if(esp->type != eax->type){
					eax->ptr = OBJECT_REALLOC(eax->ptr, void *);
				}
                eax->ptr = (class_t *)esp->ptr;
            }

            eax->type = esp->type;

            c = c->next;
            continue;
        }
        else if (op == PUSH){
          // push the value of eax onto the stack_frame
          table_rpush(frame, (value_p)eax);
		  eax = nullptr;
          c = c->next;
          continue;
        }
        else if (op == POP){
          // push the value of eax onto the stack_frame
          rp = table_rpop(frame);
          if(!(esp = (object_t *)rp->value)){
            printf("bad pop data!\n");
            exit(-1);
          }
          eax = esp;
          c = c->next;
          continue;
        }
        else if (op == JMP){
          // jump to the address
          c = c->next;
          c = (iarray_t *)c->value;
          continue;
        }
        else if (op == JZ){
          // jump if eax is zero
          c = c->next;
          c = (eax->num) ? c->next : (iarray_t *)c->value;
          continue;
        }
        else if (op == JNZ){
          // jump if eax is not zero
          c = c->next;
          c = (eax->num) ? (iarray_t *)c->value : c->next;
          continue;
        }
        else if (op == CENT){
            variable_t *var;
            if(!(var = malloc(sizeof(variable_t)))){
                printf("object, bad memory!\n");
                exit(-1);
            }

            var->key = epx->key;
			if(!(var->value = OBJECT_MALLOC(void *))){
		        printf("object, bad memory!\n");
		        exit(-1);
		    }
            var->value->type = TP_CLASS;
            var->value->ptr = epx;

            table_rpush(epx->variables, (value_p)var);

			//printf("'this' in %s\n", epx->key);

            if(!(var = malloc(sizeof(variable_t)))){
                printf("object, bad memory!\n");
                exit(-1);
            }

            var->key = epx->super->key;
			if(!(var->value = OBJECT_MALLOC(void *))){
		        printf("object, bad memory!\n");
		        exit(-1);
		    }
            var->value->type = TP_CLASS;
            var->value->ptr = epx->super;

            table_rpush(epx->variables, (value_p)var);

			//printf("'super' in %s\n", epx->key);

			epx->type = CLASS_BURN;

            c = c->next;
            continue;
        }
        else if (op == CLEV){
            rp = table_rpop(stack_efx);
            efx = (function_t *)rp->value;

            rp = table_rpop(stack_epx);
            epx = (class_t *)rp->value;

            rp = table_rpop(stack_frame);
            frame = (table_t *)rp->value;

            if(!frame){
                printf("point class leave, free frame!\n");
                exit(-1);
            }

            rp = table_rpop(frame);
            c = (iarray_t *)rp->value;
            continue;
        }
        else if (op == CALL){
            // call subroutine
			variable_t *var = nullptr;
			if((rp = table_rpop(stack_edx))){
				var = (variable_t *)rp->value;
			}

			rp = table_rpop(frame);
            esp = (object_t *)rp->value;

            long64_t i;
            long64_t cnt = esp ? esp->num : 0;

			table_t *newframe = table_create();

            for(i = 0; i < cnt; i++){
                rp = table_rpop(frame);
                table_rpush(newframe, rp->value);
            }

            table_rpush(stack_efx, (value_p)efx);

			if((rp = table_rpop(stack_ecx))){
				ecx = (class_t *)rp->value;
			}

            if(!(efx = function_get(ecx ? ecx : epx ,var->key ,cnt, FN_PAREN))){
                printf("call %s %s.%s(%lld parameters) not define this function!\n",
                    ecx ? "ecx" : "epx", ecx ? ecx->key : epx->key, var->key, cnt);
                exit(-1);
            }

            ecx = nullptr;

            table_rpush(frame, (value_p)c->next);
            table_rpush(stack_frame, (value_p)frame);

            frame = newframe;

            c = (iarray_t *)efx->start;
            continue;
        }
        else if (op == ENT){
            table_rpush(stack_epx, (value_p)epx);
            epx = efx->super;
            c = c->next;
            continue;
        }
        else if (op == LEV){
            /*
			if(efx){
                function_delete(efx);
            }
			*/

			if(efx){
				if(strncmp(epx->key, efx->key, max(strlen(epx->key), strlen(efx->key))) == 0){
					if(!(egx = variable_get(epx->variables, epx->key))){
		                printf("'this' object not defined in class %s.\n", epx->key);
		                exit(-1);
		            }
		            eax = egx->value;
		        }
			}

            rp = table_rpop(stack_efx);
            efx = (function_t *)rp->value;

            rp = table_rpop(stack_epx);
            epx = (class_t *)rp->value;

            table_destroy(frame, object_destroy);

            rp = table_rpop(stack_frame);
            frame = (table_t *)rp->value;

            if(!frame){
                printf("breakpoint function leave, free frame!\n");
                exit(-1);
            }

            rp = table_rpop(frame);
            c = (iarray_t *)rp->value;
            continue;
        }
        else if (op == THIS){
            if(!(egx = variable_get(epx->variables, epx->key))){
                printf("'this' object not defined in class %s.\n", epx->key);
                exit(-1);
            }
            eax = egx->value;
            c = c->next;
            continue;
        }
        else if (op == SUPER){
            if(!(egx = variable_get(epx->variables, epx->super->key))){
                printf("'super' object not defined.\n");
                exit(-1);
            }
            eax = egx->value;
            c = c->next;
            continue;
        }
        else if (op == CHG){
			if(eax->type == TP_CLASS){
				if((ecx = (class_t *)eax->ptr)){
					table_rpush(stack_ecx, (value_p)ecx);
					if(ecx->type == CLASS_RAW){
						table_t *newframe = table_create();

						table_rpush(stack_epx, (value_p)epx);
						epx = ecx;

						table_rpush(stack_efx, (value_p)efx);
			            efx = nullptr;

			            table_rpush(frame, (value_p)c->next);
			            table_rpush(stack_frame, (value_p)frame);
			            frame = newframe;

			            c = (iarray_t *)epx->start;
			            continue;
					}
				}
			}
            c = c->next;
            continue;
        }
        else if (op == SIZEOF){
            if(eax->type == TP_NULL){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                obj->type = TP_NUMBER;
                obj->num = object_sizeof(eax);
                eax = obj;
                c = c->next;
                continue;
            }
			else if(eax->type == TP_CHAR){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                obj->type = TP_NUMBER;
                obj->num = object_sizeof(eax);
                eax = obj;
                c = c->next;
                continue;
            }
			else if(eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                obj->type = TP_NUMBER;
                obj->num = object_sizeof(eax);
                eax = obj;
                c = c->next;
                continue;
            }
            else if(eax->type == TP_DATA){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                obj->type = TP_NUMBER;
                obj->num = data_sizeof((table_t *)eax->ptr);
                eax = obj;
                c = c->next;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);

                if(!(efx = function_get((class_t *)eax->ptr , "sizeof",1, FN_PAREN))){
                  printf("operator 'sizeof' not define in this function!\n");
                  exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                obj->type = TP_NUMBER;
                obj->num = 1;

                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("sizeof, unknown type!\n");
            exit(-1);
        }
        else if (op == TYPEOF){
            if(eax->type == TP_NULL){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                obj->type = TP_DATA;
                obj->ptr = data_from(STR_NULL);
                eax = obj;
                c = c->next;
                continue;
            }
            else if(eax->type == TP_CHAR){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                obj->type = TP_DATA;
                obj->ptr = data_from(STR_CHAR);
                eax = obj;
                c = c->next;
                continue;
            }
            else if(eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                obj->type = TP_DATA;
                obj->ptr = data_from(STR_NUMBER);
                eax = obj;
                c = c->next;
                continue;
            }
            else if(eax->type == TP_DATA){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                obj->type = TP_DATA;
                obj->ptr = data_from(STR_DATA);
                eax = obj;
                c = c->next;
                continue;
            }
            else if(eax->type == TP_CLASS){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                obj->type = TP_DATA;
                obj->ptr = class_get_name((class_t *)eax->ptr);
                eax = obj;
                c = c->next;
                continue;
            }

            printf("eval: this type not defined.\n");
            exit(-1);
        }
        else if (op == DELETE){
            c = c->next;

			table_t *newframe = table_create();
			long64_t i;
			for(i = 0; i < c->value; i++){
				rp = table_rpop(frame);
				table_rpush(newframe, rp->value);
			}

			rp = table_rpop(frame);
			esp = (object_t *)rp->value;

            if(esp->type == TP_DATA){
                table_t *tbl = (table_t *)esp->ptr;
				table_sort(newframe, object_sort);
				i = -1;
                while((rp = table_lpop(newframe))){
                    esp = (object_t *)rp->value;
					if((i == esp->num) || (esp->num < 0)){
						object_delete(esp);
						continue;
					}
					i = esp->num;
                    itable_t *t = table_at(tbl, i);
                    if(t->value){
                        object_delete((object_t *)t->value);
                    }
                    table_unlink(tbl, t);
                    object_delete(esp);
                }
            }
            else {
                object_delete(esp);
                while((rp = table_rpop(newframe))){
                    object_delete((object_t *)rp->value);
                }
            }

            c = c->next;
            continue;
        }
        else if (op == COUNT){
			if(eax->type == TP_NULL){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                obj->type = TP_NUMBER;
                obj->num = 1;
                eax = obj;
                c = c->next;
                continue;
            }
			if(eax->type == TP_CHAR){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                obj->type = TP_NUMBER;
                obj->num = 1;
                eax = obj;
                c = c->next;
                continue;
            }
            if(eax->type == TP_NUMBER){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                obj->type = TP_NUMBER;
                obj->num = 1;
                eax = obj;
                c = c->next;
                continue;
            }
            else if(eax->type == TP_DATA){
                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }
                obj->type = TP_NUMBER;
                table_t *tbl = (table_t *)eax->ptr;
                obj->num = table_count(tbl);
                eax = obj;
                c = c->next;
                continue;
            }
            else if(eax->type == TP_CLASS){
                table_rpush(stack_efx, (value_p)efx);
                if(!(efx = function_get((class_t *)eax->ptr , "count",1, FN_PAREN))){
                    printf("eval: operator '$' not define in this function!\n");
                    exit(-1);
                }

                table_t *newframe = table_create();
                table_rpush(newframe, (value_p)eax);

                object_t *obj;
                if(!(obj = OBJECT_MALLOC(double))){
                    printf("object, bad memory!\n");
                    exit(-1);
                }

                obj->type = TP_NUMBER;
                obj->num = 1;
                table_rpush(newframe, (value_p)obj);

                table_rpush(frame, (value_p)c->next);
                table_rpush(stack_frame, (value_p)frame);

                frame = newframe;

                c = (iarray_t *)efx->start;
                continue;
            }

            printf("eval: this type not defined.\n");
            exit(-1);
        }
        else if (op == RST){
            ecx = nullptr;
            c = c->next;
            continue;
        }
		else if (op == SVPA){
            table_rpush(stack_edx, (value_p)egx);
            c = c->next;
            continue;
        }
		else if (op == NEW){
			if(eax->type == TP_CLASS){
				if((ecx = (class_t *)eax->ptr)){
					table_t *newframe = table_create();

					table_rpush(stack_epx, (value_p)epx);
					epx = (ecx->type == CLASS_RAW) ? ecx : class_clone(ecx);
					table_rpush(stack_ecx, (value_p)epx);

					table_rpush(stack_efx, (value_p)efx);
					efx = nullptr;

					table_rpush(frame, (value_p)c->next);
					table_rpush(stack_frame, (value_p)frame);
					frame = newframe;

					c = (iarray_t *)epx->start;
					continue;
				}
			}
            c = c->next;
            continue;
        }
        else if (op == SVBR){
            table_rpush(stack_ebx, (value_p)egx);
            c = c->next;
            continue;
        }
		else if (op == REF){
            egx->value->level = LEVEL_REFRENCE;
            c = c->next;
            continue;
        }
		else if (op == RET){
			if(eax){
				eax->level = LEVEL_RESPONSE;
			}
			c = c->next;
			continue;
		}
		else if (op == BREAK){
			iarray_t *b = c->next;
            while(b != code->begin){
                if(b->value == LOPE){
                    c = b;
                    break;
                }
                b = b->next;
            }

			c = c->next;
            continue;
		}


        else if (op == PRTF){
            c = c->next;
            table_t *parameters = table_create();

            long64_t i;
            for(i = 0; i < c->value; i++){
                rp = table_rpop(frame);
                if(!(esp = (object_t *)rp->value)){
                    printf("eval: bad pop stack.\n");
                    exit(-1);
                }
                table_lpush(parameters, (value_p)esp);
            }

			rp = table_rpop(frame);
			if(!(eax = (object_t *)rp->value)){
				printf("eval: bad pop stack.\n");
				exit(-1);
			}

			if(eax->type == TP_NULL){
				printf("%s ", STR_NULL);
				c = c->next;
	            continue;
			}
			else if(eax->type == TP_DATA){
				table_t *formated;
				if(i > 0){
					formated = table_create();
					data_format((table_t *)eax->ptr, parameters, formated);
				}
				else {
					formated = (table_t *)eax->ptr;
				}

				itable_t *t = formated->begin;
				while( t != formated->end ){
					object_t *obj = (object_t *)t->value;
					if(obj->type != TP_CHAR){
						printf("formated string must created of chars!\n");
			            exit(-1);
					}

					if(obj->num == '\\'){
						t = t->next;
						obj = (object_t *)t->value;

						if(obj->num == 'n'){
							printf("\n");
							t = t->next;
							continue;
						}
						else
						if(obj->num == 't'){
							printf("\t");
							t = t->next;
							continue;
						}
						else
						if(obj->num == 'v'){
							printf("\v");
							t = t->next;
							continue;
						}

						printf("\\");
						t = t->next;
						continue;
					}

					printf("%c", (char)obj->num);
					t = t->next;
				}

				if(i > 0){
					data_delete(formated);
				}

				c = c->next;
	            continue;
			}
			else if(eax->type == TP_CHAR){
				printf("%c", (char)eax->num);
				c = c->next;
	            continue;
			}
			else if(eax->type == TP_NUMBER){
				if((eax->num - (long64_t)eax->num) != 0){
					printf("%.16f", eax->num);
				}else{
					printf("%lld", (long64_t)eax->num);
				}
				c = c->next;
	            continue;
			}
			else if(eax->type == TP_CLASS){
				class_t *cls = (class_t *)eax->ptr;
				printf("%s", cls->key);
				c = c->next;
	            continue;
			}
			else {
				printf("%.16f", eax->num);
			}

            c = c->next;
            continue;
        }
		else if (op == FORMAT){
			c = c->next;
            table_t *parameters = table_create();

            long64_t i;
            for(i = 0; i < c->value; i++){
                rp = table_rpop(frame);
                if(!(esp = (object_t *)rp->value)){
                    printf("eval: bad pop stack.\n");
                    exit(-1);
                }
                table_lpush(parameters, (value_p)esp);
            }

			rp = table_rpop(frame);
			if(!(eax = (object_t *)rp->value)){
				printf("eval: bad pop stack.\n");
				exit(-1);
			}

			if(eax->type != TP_DATA){
				printf("first parameter of format not a string!\n");
				exit(-1);
			}

			table_t *formated = table_create();
			data_format((table_t *)eax->ptr, parameters, formated);

			if(!(eax = OBJECT_MALLOC(void *))){
				printf("object, bad memory!\n");
				exit(-1);
			}
			eax->type = TP_DATA;
			eax->ptr = formated;

			c = c->next;
            continue;
		}

		else if (op == OPEN){
			rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
				printf("open, bad pop object!\n");
                exit(-1);
			}

			if(esp->type != TP_DATA){
				printf("open, path must be a string!\n");
                exit(-1);
			}

			char *path = data_to ((table_t *)esp->ptr);
			path[table_count((table_t *)esp->ptr)] = '\0';

			if(eax->type != TP_NUMBER){
				printf("open, object not a number!\n");
                exit(-1);
			}

			// O_WRONLY | O_RDWR | O_RDONLY | O_CREAT
			int flag = (int)eax->num;

			object_t *obj;
            if(!(obj = OBJECT_MALLOC(double))){
                printf("object, bad memory!\n");
                exit(-1);
            }

			obj->type = TP_NUMBER;
			obj->num = open(path, flag);

			FREE(path);

			object_delete (esp);
			object_delete (eax);

			eax = obj;

			c = c->next;
            continue;
        }
		else if (op == CLOSE){
			if(eax->type != TP_NUMBER){
				printf("close, object not a number!\n");
                exit(-1);
			}

			object_t *obj;
            if(!(obj = OBJECT_MALLOC(double))){
                printf("object, bad memory!\n");
                exit(-1);
            }
			obj->type = TP_NUMBER;
			obj->num = close(eax->num);

			object_delete (eax);

			eax = obj;

			c = c->next;
            continue;
		}
		else if (op == READ){
			if(eax->type != TP_NUMBER){
				printf("read, object not a number!\n");
				exit(-1);
			}

			char *buf = MALLOC(eax->num * sizeof(char));

			object_t *obj_buf;
			rp = table_rpop(frame);
            if(!(obj_buf = (object_t *)rp->value)){
				printf("read, buffer is null!\n");
                exit(-1);
			}
			obj_buf->type = TP_DATA;

			rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
				printf("read, buffer is null!\n");
                exit(-1);
			}

			object_t *obj;
            if(!(obj = OBJECT_MALLOC(double))){
                printf("object, bad memory!\n");
                exit(-1);
            }
			obj->type = TP_NUMBER;
			obj->num = read(esp->num, buf, eax->num);

			obj_buf->ptr = data_from (buf);
			FREE(buf);

			object_delete (eax);

			eax = obj;

			c = c->next;
            continue;
        }
		else if (op == WRITE){
			rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
				printf("write, buffer is null!\n");
                exit(-1);
			}

			char *buf = data_to ((table_t *)esp->ptr);

			rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
				printf("write, buffer is null!\n");
                exit(-1);
			}

			if(eax->type != TP_NUMBER){
				printf("write, object not a number!\n");
                exit(-1);
			}

			object_t *obj;
            if(!(obj = OBJECT_MALLOC(double))){
                printf("object, bad memory!\n");
                exit(-1);
            }
			obj->type = TP_NUMBER;
			obj->num = write(esp->num, buf, eax->num);

			FREE(buf);

			object_delete (eax);
			object_delete (esp);

			eax = obj;

			c = c->next;
            continue;
        }
		else if (op == SEEK){
			rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
				printf("seek, buffer is null!\n");
                exit(-1);
			}

			long n = esp->num;

			rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
				printf("seek, buffer is null!\n");
                exit(-1);
			}

			if(eax->type != TP_NUMBER){
				printf("seek, object not a number!\n");
                exit(-1);
			}

			object_t *obj;
            if(!(obj = OBJECT_MALLOC(double))){
                printf("object, bad memory!\n");
                exit(-1);
            }
			obj->type = TP_NUMBER;
			obj->num = lseek(esp->num, n, eax->num);

			object_delete (eax);
			object_delete (esp);

			eax = obj;

			c = c->next;
            continue;
        }

		else if (op == RENAME){
			rp = table_rpop(frame);
            if(!(esp = (object_t *)rp->value)){
				printf("rename, bad pop object!\n");
                exit(-1);
			}

			if(esp->type != TP_DATA){
				printf("rename, path must be a string!\n");
                exit(-1);
			}

			char *path = data_to ((table_t *)esp->ptr);
			path[table_count((table_t *)esp->ptr)] = '\0';

			if(eax->type != TP_DATA){
				printf("rename, object not a string!\n");
                exit(-1);
			}

			char *path2 = data_to ((table_t *)eax->ptr);
			path2[table_count((table_t *)eax->ptr)] = '\0';

			object_t *obj;
            if(!(obj = OBJECT_MALLOC(double))){
                printf("object, bad memory!\n");
                exit(-1);
            }

			obj->type = TP_NUMBER;
			obj->num = rename(path, path2);

			FREE(path);
			FREE(path2);

			object_delete (esp);
			object_delete (eax);

			eax = obj;

			c = c->next;
            continue;
        }
		else if (op == CWD){
			object_t *obj;
			char cwd[MAX_PATH];
			if (getcwd(cwd, sizeof(cwd)) != NULL) {
				if(!(obj = OBJECT_MALLOC(void *))){
					printf("object, bad memory!\n");
					exit(-1);
				}
				obj->type = TP_DATA;
				obj->ptr = data_from(cwd);
			}else{
				if(!(obj = OBJECT_MALLOC(void *))){
					printf("object, bad memory!\n");
					exit(-1);
				}
				obj->type = TP_DATA;
				obj->ptr = data_from(STR_NULL);
			}

			eax = obj;

			c = c->next;
            continue;
        }
		else if (op == CHDIR){
			if(eax->type != TP_DATA){
				printf("chdir, object not a string!\n");
                exit(-1);
			}

			char *path = data_to ((table_t *)eax->ptr);

			object_t *obj;
            if(!(obj = OBJECT_MALLOC(double))){
                printf("object, bad memory!\n");
                exit(-1);
            }

			obj->type = TP_NUMBER;
			obj->num = chdir(path);

			FREE(path);

			object_delete (eax);

			eax = obj;

			c = c->next;
            continue;
        }
		else if (op == GETS){
			object_t *obj;

			if(!(obj = OBJECT_MALLOC(void *))){
				printf("object, bad memory!\n");
				exit(-1);
			}

			if(eax->type != TP_NUMBER){
				printf("gets, object not a string!\n");
				exit(-1);
			}

			char *str = MALLOC(sizeof(char) * (long64_t)eax->num);
			fflush(stdin);

			obj->type = TP_DATA;
			obj->ptr = data_from(fgets(str,MAX_PATH,stdin));

			eax = obj;

			c = c->next;
			continue;
        }
		else if (op == GETC){
			object_t *obj;

			if(!(obj = OBJECT_MALLOC(void *))){
				printf("object, bad memory!\n");
				exit(-1);
			}

			char str[2];

			fflush(stdin);

			str[0] = fgetc(stdin);
			str[1] = '\0';

			obj->type = TP_DATA;
			obj->ptr = data_from(str);

			eax = obj;

			c = c->next;
			continue;
        }

		else if (op == TICK){
			object_t *obj;
			if(!(obj = OBJECT_MALLOC(double))){
				printf("object, bad memory!\n");
				exit(-1);
			}
			obj->type = TP_NUMBER;
			obj->num = (long64_t)abs(clock());

			eax = obj;

			c = c->next;
            continue;
        }
		else if (op == TIME){
			object_t *obj;
			if(!(obj = OBJECT_MALLOC(double))){
				printf("object, bad memory!\n");
				exit(-1);
			}
			obj->type = TP_NUMBER;
			obj->num = (long64_t)time(NULL);

			eax = obj;

			c = c->next;
			continue;
		}

		else if (op == SETUP){
			c = c->next;

			if(!(epx = class_get(base, (char*)c->value))){
				printf("eval: in new operator class %s not definition!\n", (char *)c->value);
				exit(-1);
			}

			frame = table_create();

			ecx = nullptr;
			c = (iarray_t *)epx->start;
			continue;
		}
		else if (op == EXIT){
            break;
        }
        else {
            printf("unknown instruction:%lld\n", op);
            exit(-1);
        }
    }
}


int
main(int argc, char **argv)
{
    argc--;
    argv++;

    // parse arguments
    if (argc < 1) {
        printf("usage: file ...\n");
        return -1;
    }

    long64_t i;
    FILE *fd;

	char destination [ MAX_PATH ];

#ifdef WIN32
	if(**argv != '\\'){
#else
	if(**argv != '/'){
#endif
		char cwd[MAX_PATH];
		if (getcwd(cwd, sizeof(cwd)) == NULL) {
			perror("getcwd() error");
			return -1;
		}
		utils_combine ( destination, cwd, *argv );
	} else {
		strcpy(destination, *argv);
	}

    if (!(fd = fopen(destination, "rb"))) {
        printf("could not open(%s)\n", destination);
        return -1;
    }

    argc--;
    argv++;

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
        return -1;
    }

    // read the source file
    if ((i = fread(buf, 1, chunk, fd)) < chunk) {
        printf("read returned %lld\n", i);
        return -1;
    }

    buf[i] = '\0';

    fclose(fd);

    table_t *tokens = table_create();
    lexer(tokens, buf);
    FREE(buf);

    class_t *base;
    array_t *code = array_create();
    base = parser(tokens, argc, argv, code);

    eval(base, code);

    table_destroy(tokens, token_destroy);
    return 0;
}
