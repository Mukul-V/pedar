#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <dirent.h>

#ifdef WIN32
	#include <conio.h>
#else
	#include "conio.h"
#endif

#include "types.h"
#include "utils.h"
#include "array.h"
#include "table.h"
#include "lexer.h"
#include "parser.h"
#include "memory.h"
#include "vm.h"

static char *STR_NULL = "NULL";
static char *STR_CHAR = "CHAR";
static char *STR_NUMBER = "NUMBER";
static char *STR_DATA = "DATA";


#define pedar_object_malloc(sz) pedar_malloc(sizeof(object_type_t) + sizeof(object_level_t) + sz)
#define pedar_object_realloc(ptr, sz) pedar_realloc(ptr, sizeof(object_type_t) + sizeof(object_level_t) + sz)


/* variable type */
typedef struct variable {
	string_t key;
	object_t *value;
} variable_t;

//OR LOR XOR AND LAND EQ NE LT LE GT GE LSHIFT RSHIFT ADD SUB MUL DIV MOD


function_t *
function_get(class_t *clspar, char *key, value_t nparams, fn_type_t type);

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

value_t
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



value_t
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



value_t
object_destroy(itable_t *it);

void
object_delete(object_t *obj);

value_t
object_sizeof(object_t *obj);

value_t
object_sort(value_t *obj_1, value_t *obj_2){
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
			pedar_free(obj);
		}
        return;
    }
	else if(obj->type == TP_NUMBER){
		if(obj->level == LEVEL_TEMPRATORY){
			pedar_free(obj);
		}
        return;
    }
	else if(obj->type == TP_CHAR){
		if(obj->level == LEVEL_TEMPRATORY){
			pedar_free(obj);
		}
        return;
    }
    else if(obj->type == TP_DATA){
		if(obj->level == LEVEL_TEMPRATORY){
			//data_delete((table_t *)obj->ptr);
			//pedar_free(obj);
		}
        return;
    }
    else if(obj->type == TP_CLASS){
		if(obj->level == LEVEL_TEMPRATORY){
			//class_delete((class_t *)obj->ptr);
			//pedar_free(obj);
		}
        return;
    }
}

value_t
object_destroy(itable_t *it)
{
	object_t *obj;
	if(!(obj = (object_t *)it->value)){
		printf("free frame, object not defined!\n");
		exit(-1);
	}
    //object_delete(obj);
    pedar_free(it);
    return 1;
}

value_t
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
    object_t *res = pedar_object_malloc(sizeof(double));
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
function_get(class_t *clspar, char *key, value_t nparams, fn_type_t type)
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
    pedar_free(fnpar);
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

    pedar_free(clspar);
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
			obj = pedar_object_malloc(sizeof(double));
			obj->type = TP_CHAR;
			obj->num = '.';
			table_lpush(tbl, (value_p)obj);
		}
		value_t i = 0;
		for(i = strlen(clspar->key); i > 0; i--){
			obj = pedar_object_malloc(sizeof(double));
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

value_t
class_sizeof(class_t *clspar)
{
	value_t res = 0;
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
    if(!(var = pedar_malloc(sizeof(variable_t)))){
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
    pedar_free(var);
}



value_t
data_sizeof(table_t *tbl)
{
    value_t res = 0;
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
    value_t i;
    for(i = 0; i < strlen(str); i++){
        object_t *obj;
        if(!(obj = pedar_object_malloc(sizeof(char)))){
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
    value_t i = 0;
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
data_format(table_t *tbl, table_t *format, table_t *formated)
{
	itable_t *t = tbl->begin, *f;
	object_t *esp, *obj;

    while( t != tbl->end ){
        obj = (object_t *)t->value;

        if(obj->type == TP_NULL){
			value_t i;
			for(i = 0; i < strlen(STR_NULL); i++){
				if(!(esp = pedar_object_malloc(sizeof(char)))){
                    printf("unable to alloc memory!\n");
                    exit(-1);
                }
                esp->type = TP_CHAR;
                esp->num = (char)STR_NULL[i];
				table_rpush(formated, (value_p)esp);
			}
			if(!(esp = pedar_object_malloc(sizeof(char)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			esp->type = TP_CHAR;
			esp->num = ' ';
			table_rpush(formated, (value_p)esp);
            t = t->next;
            continue;
        }
		else
		if(obj->type == TP_CHAR){
			value_t num = 0;
			if(obj->num == '%'){
				t = t->next;
				obj = (object_t *)t->value;

				if(obj->num == 's'){
					if(!(f = table_rpop(format))){
						printf("format, %%s require a string data!\n");
						exit(-1);
					}
					obj = (object_t *)f->value;
					if(obj->type != TP_DATA){
						printf("%%s must be input string data!\n");
						exit(-1);
					}
					data_format((table_t *)obj->ptr, format, formated);
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
					if(!(f = table_rpop(format))){
						printf("format, %%n require a number!\n");
						exit(-1);
					}

					obj = (object_t *)f->value;

					char *fmt = pedar_malloc(sizeof(char) * 255);
					sprintf(fmt, "%%.%lldf", num);

					char *str_num = pedar_malloc(sizeof(char) * 255);
					if(((obj->num - (value_t)obj->num) != 0) || (num > 0)){
						sprintf(str_num, fmt, obj->num);
					}
					else {
						sprintf(str_num, "%lld", (value_t)obj->num);
					}

					pedar_free(fmt);

					value_t i;
					for(i = 0; i < strlen(str_num); i++){
						if(!(esp = pedar_object_malloc(sizeof(char)))){
							printf("unable to alloc memory!\n");
							exit(-1);
						}
						esp->type = TP_CHAR;
						esp->num = (char)str_num[i];
						table_rpush(formated, (value_p)esp);
					}

					pedar_free(str_num);

					t = t->next;
					continue;
				}
				else
				if(obj->num == 'h'){
					if(!(f = table_rpop(format))){
						printf("format, %%n require a number!\n");
						exit(-1);
					}

					obj = (object_t *)f->value;

					char *fmt = pedar_malloc(sizeof(char) * 255);
					sprintf(fmt, "%%0%lldllx", num);

					char *str_num = pedar_malloc(sizeof(char) * 255);
					if(((obj->num - (value_t)obj->num) != 0) || (num > 0)){
						sprintf(str_num, fmt, obj->num);
					}
					else {
						sprintf(str_num, "%llx", (value_t)obj->num);
					}

					pedar_free(fmt);

					value_t i;
					for(i = 0; i < strlen(str_num); i++){
						if(!(esp = pedar_object_malloc(sizeof(char)))){
							printf("unable to alloc memory!\n");
							exit(-1);
						}
						esp->type = TP_CHAR;
						esp->num = (char)str_num[i];
						table_rpush(formated, (value_p)esp);
					}

					pedar_free(str_num);

					t = t->next;
					continue;
				}
				else
				if(obj->num == 'H'){
					if(!(f = table_rpop(format))){
						printf("format, %%n require a number!\n");
						exit(-1);
					}

					obj = (object_t *)f->value;

					char *fmt = pedar_malloc(sizeof(char) * 255);
					sprintf(fmt, "%%0%lldllX", num);

					char *str_num = pedar_malloc(sizeof(char) * 255);
					if(((obj->num - (value_t)obj->num) != 0) || (num > 0)){
						sprintf(str_num, fmt, obj->num);
					}
					else {
						sprintf(str_num, "%llX", (value_t)obj->num);
					}

					pedar_free(fmt);

					value_t i;
					for(i = 0; i < strlen(str_num); i++){
						if(!(esp = pedar_object_malloc(sizeof(char)))){
							printf("unable to alloc memory!\n");
							exit(-1);
						}
						esp->type = TP_CHAR;
						esp->num = (char)str_num[i];
						table_rpush(formated, (value_p)esp);
					}

					pedar_free(str_num);

					t = t->next;
					continue;
				}

				if(!(esp = pedar_object_malloc(sizeof(char)))){
					printf("unable to alloc memory!\n");
					exit(-1);
				}
				esp->type = TP_CHAR;
				esp->num = obj->num;
				table_rpush(formated, (value_p)esp);

				t = t->next;
				continue;
			}

			if(!(esp = pedar_object_malloc(sizeof(char)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			esp->type = TP_CHAR;
			esp->num = obj->num;
			table_rpush(formated, (value_p)esp);
			t = t->next;
			continue;
		}
		else
		if(obj->type == TP_CLASS){
			class_t *cls = (class_t *)obj->ptr;
			value_t i;
			for(i = 0; i < strlen(cls->key); i++){
				if(!(esp = pedar_object_malloc(sizeof(char)))){
					printf("unable to alloc memory!\n");
					exit(-1);
				}
				esp->type = TP_CHAR;
				esp->num = (char)cls->key[i];
				table_rpush(formated, (value_p)esp);
			}
			if(!(esp = pedar_object_malloc(sizeof(char)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			esp->type = TP_CHAR;
			esp->num = ' ';
			table_rpush(formated, (value_p)esp);
			t = t->next;
			continue;
		}
		else
		if(obj->type == TP_NUMBER){
			char *str_num = pedar_malloc(sizeof(char) * 255);
			if((obj->num - (value_t)obj->num) != 0){
				sprintf(str_num, "%.16f", obj->num);
			}
			else {
				sprintf(str_num, "%lld", (value_t)obj->num);
			}
			value_t i;
			for(i = 0; i < strlen(str_num); i++){
				if(!(esp = pedar_object_malloc(sizeof(char)))){
					printf("unable to alloc memory!\n");
					exit(-1);
				}
				esp->type = TP_CHAR;
				esp->num = (char)str_num[i];
				table_rpush(formated, (value_p)esp);
			}

			pedar_free(str_num);

			if(!(esp = pedar_object_malloc(sizeof(char)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			esp->type = TP_CHAR;
			esp->num = ' ';
			table_rpush(formated, (value_p)esp);
			t = t->next;
			continue;
		}
		else
		if(obj->type == TP_DATA){
			data_format((table_t *)obj->ptr, format, formated);
			t = t->next;
			continue;
		}

		printf("this type not a acceptable!\n");
		exit(-1);
	}

	return formated;
}

void
data_delete(table_t *tbl)
{
    itable_t *b, *n;
    for( b = tbl->begin; b != tbl->end; b = n ){
        n = b->next;
        object_delete((object_t *)b->value);
        pedar_free(b);
    }
    pedar_free(tbl);
}

value_t
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
			value_t r = data_compare(tn1,tn2);
			data_delete(tn1);
			data_delete(tn2);
			return r;
		}

		t1 = t1->next;
		t2 = t2->next;
	}

	return 1;
}

typedef struct context {
	/* class save refrence of parrent class */
	class_t *class;
	table_t *table_class;

	/* fn save refrence of path func in variables */
	function_t *fn;
	table_t *table_fn;

	class_t *floating;
	table_t *table_floating;

	frame_t *frame;
	table_t *table_frame;

	table_t *table_be;
	table_t *table_ne;

	variable_t 	*variable;
	object_t *object;

	class_t *base;
	array_t *code;
} context_t;

context_t *
context_update(
	context_t *ctx,
	class_t *class,
	table_t *table_class,
	function_t *fn,
	table_t *table_fn,
	class_t *floating,
	table_t *table_floating,
	frame_t *frame,
	table_t *table_frame,
	table_t *table_be,
	table_t *table_ne,
	variable_t *variable,
	object_t *object,
	class_t *base,
	array_t *code){

	ctx->class = class;
	ctx->table_class = table_class;
	ctx->fn = fn;
	ctx->table_fn = table_fn;
	ctx->floating = floating;
	ctx->table_floating = table_floating;
	ctx->frame = frame;
	ctx->table_frame = table_frame;
	ctx->table_be = table_be;
	ctx->table_ne = table_ne;
	ctx->variable = variable;
	ctx->object = object;
	ctx->base = base;
	ctx->code = code;

	return ctx;
}

iarray_t *
fn_prepare(context_t *ctx, iarray_t *c, class_t *class, char *name, value_t nparam, fn_type_t type){
	// save current fn
	table_rpush(ctx->table_fn, (value_p)ctx->fn);

	// create entry for new fn
	value_t i;
	frame_t *frame2 = frame_create();
	for(i = 0; i < nparam; i++){
		frame_push(frame2, frame_pop(ctx->frame));
	}

	// create return point
	object_t *obj;
	if(!(obj = pedar_object_malloc(sizeof(double)))){
		printf("unable to prepare function!\n");
		exit(-1);
	}
	obj->type = TP_ADRS;
	obj->ptr = (value_p)c->next;

	frame_push(ctx->frame, obj);
	table_rpush(ctx->table_frame, (value_p)ctx->frame);

	ctx->frame = frame2;

	if(!(ctx->fn = function_get(class, name, nparam, type))){
		printf("unable to found function '%s' in class %s!\n", name, class->key);
		exit(-1);
	}

	ctx->floating = nullptr;

	return (iarray_t *)ctx->fn->start;
}

iarray_t *
decode_or(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[||], you can use of '||' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[or] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = (value_t)esp->num || (value_t)ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, "||", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, "||", 1, FN_PAREN);
	}

	printf("[||] operator not defined!\n");
	exit(-1);
}

iarray_t *
decode_lor(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[|], you can use of '|' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[lor] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = (value_t)esp->num | (value_t)ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, "|", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, "|", 1, FN_PAREN);
	}

	printf("[|] operator not defined!\n");
	exit(-1);
}

iarray_t *
decode_xor(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[^], you can use of '^' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[^] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = (value_t)esp->num ^ (value_t)ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, "^", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, "^", 1, FN_PAREN);
	}

	printf("[^] operator not defined!\n");
	exit(-1);
}

iarray_t *
decode_and(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[&&], you can use of '&&' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[&&] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = (value_t)esp->num && (value_t)ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, "&&", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, "&&", 1, FN_PAREN);
	}

	printf("[&&] operator not defined!\n");
	exit(-1);
}

iarray_t *
decode_land(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[&], you can use of '&' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[&] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = (value_t)esp->num & (value_t)ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, "&", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, "&", 1, FN_PAREN);
	}

	printf("[&] operator not defined!\n");
	exit(-1);
}

iarray_t *
decode_eq(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[==], you can use of '==' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[or] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = esp->num == ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, "==", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, "==", 1, FN_PAREN);
	}
	else if(esp->type == TP_DATA && ctx->object->type == TP_DATA){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[==] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = data_compare((table_t *)esp->ptr, (table_t *)ctx->object->ptr);

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}

	printf("[==] operator not defined!\n");
	exit(-1);
}

iarray_t *
decode_ne(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[!=], you can use of '!=' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[!=] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = esp->num != ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, "!=", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, "!=", 1, FN_PAREN);
	}
	else if(esp->type == TP_DATA && ctx->object->type == TP_DATA){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[!=] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = !data_compare((table_t *)esp->ptr, (table_t *)ctx->object->ptr);

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}

	printf("[!=] operator not defined %d %d !\n", ctx->object->type, esp->type);
	exit(-1);
}

iarray_t *
decode_lt(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[<], you can use of '<' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[<] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = esp->num < ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, "<", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, "<", 1, FN_PAREN);
	}

	printf("[<] operator not defined!\n");
	exit(-1);
}

iarray_t *
decode_le(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[<=], you can use of '<=' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[<=] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = esp->num <= ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, "<=", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, "<=", 1, FN_PAREN);
	}

	printf("[<=] operator not defined!\n");
	exit(-1);
}

iarray_t *
decode_gt(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[>], you can use of '>' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[>] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = esp->num > ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, ">", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, ">", 1, FN_PAREN);
	}

	printf("[>] operator not defined!\n");
	exit(-1);
}

iarray_t *
decode_ge(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[>=], you can use of '>=' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[or] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = esp->num >= ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, ">=", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, ">=", 1, FN_PAREN);
	}

	printf("[>=] operator not defined!\n");
	exit(-1);
}

iarray_t *
decode_lshift(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[<<], you can use of '<<' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[<<] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = (value_t)esp->num << (value_t)ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, "<<", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, "<<", 1, FN_PAREN);
	}

	printf("[<<] operator not defined!\n");
	exit(-1);
}

iarray_t *
decode_rshift(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[>>], you can use of '>>' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[>>] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = (value_t)esp->num >> (value_t)ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, ">>", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, ">>", 1, FN_PAREN);
	}

	printf("[>>] operator not defined!\n");
	exit(-1);
}

iarray_t *
decode_add(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[+], you can use of '+' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[+] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = esp->num + ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, "+", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, "+", 1, FN_PAREN);
	}

	printf("[+] operator not defined!\n");
	exit(-1);
}

iarray_t *
decode_sub(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[-], you can use of '-' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[or] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = esp->num - ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, "-", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, "-", 1, FN_PAREN);
	}

	printf("[-] operator not defined!\n");
	exit(-1);
}

iarray_t *
decode_mul(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[*], you can use of '*' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[*] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = esp->num * ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, "*", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, "*", 1, FN_PAREN);
	}

	printf("[*] operator not defined!\n");
	exit(-1);
}

iarray_t *
decode_div(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[/], you can use of '/' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[/] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = esp->num / ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, "/", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, "/", 1, FN_PAREN);
	}

	printf("[/] operator not defined!\n");
	exit(-1);
}

iarray_t *
decode_mod(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("[%%], you can use of '%%' between two object!\n");
		exit(-1);
	}

	if(esp->type == TP_NUMBER && ctx->object->type == TP_NUMBER){
		object_t *obj;
		if(!(obj = pedar_object_malloc(sizeof(double)))){
			printf("[%%] object not defined!\n");
			exit(-1);
		}

		obj->type = TP_NUMBER;
		obj->num = (value_t)esp->num % (value_t)ctx->object->num;

		object_delete(esp);
		object_delete(ctx->object);

		ctx->object = obj;
		return c->next;
	}
	else if(esp->type == TP_CLASS){
		frame_push(ctx->frame, esp);
		return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, "%", 1, FN_PAREN);
	}
	else if(ctx->object->type == TP_CLASS){
		frame_push(ctx->frame, ctx->object);
		return fn_prepare(ctx, c, (class_t *)esp->ptr, "%", 1, FN_PAREN);
	}

	printf("[%%] operator not defined!\n");
	exit(-1);
}


// VAR IMM DATA SD LD PUSH JMP JZ JNZ CENT CLEV CALL ENT LEV THIS SUPER CHG
// SIZEOF TYPEOF DELETE INSERT COUNT BREAK NEW REF RET

iarray_t *
decode_var(context_t *ctx, iarray_t *c) {
	c = c->next;

	if(ctx->floating){
		class_t *rbx = nullptr;
		if((rbx = class_find(ctx->floating, (char *)c->value))){
			if(!(ctx->variable = pedar_malloc(sizeof(variable_t)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			ctx->variable->key = (char *)c->value;
			if(!(ctx->variable->value = pedar_object_malloc(sizeof(void *)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			ctx->variable->value->type = TP_CLASS;
			ctx->variable->value->level = LEVEL_REGISTER;
			ctx->variable->value->ptr = rbx;
			table_rpush(ctx->fn ? ctx->fn->variables : ctx->class->variables, (value_p)ctx->variable);
			ctx->object = ctx->variable->value;
			return c->next;
		}

		if((ctx->variable = class_getvar(ctx->floating, (char *)c->value))){
			ctx->object = ctx->variable->value;
			return c->next;
		}
	}

	if(ctx->fn){
		if((ctx->variable = variable_get(ctx->fn->variables, (char *)c->value))){
			ctx->object = ctx->variable->value;
			return c->next;
		}
	}

	if((ctx->variable = class_getvar(ctx->class, (char *)c->value))){
		ctx->object = ctx->variable->value;
		return c->next;
	}

	if(!(ctx->variable = pedar_malloc(sizeof(variable_t)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}

	class_t *rbx = nullptr;
	if(!(rbx = class_get(ctx->floating ? ctx->floating : ctx->class, (char *)c->value))){
		rbx = class_get(ctx->base, (char *)c->value);
	}

	ctx->variable->key = (char *)c->value;

	if(!(ctx->variable->value = pedar_object_malloc(sizeof(void *)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}
	ctx->variable->value->level = LEVEL_REGISTER;
	ctx->variable->value->type = rbx ? TP_CLASS : TP_NULL;
	ctx->variable->value->ptr = rbx;

	table_rpush(ctx->fn ? ctx->fn->variables : ctx->class->variables, (value_p)ctx->variable);

	ctx->object = ctx->variable->value;

	return c->next;
}

iarray_t *
decode_imm(context_t *ctx, iarray_t *c) {
	// load immediate value to object
	c = c->next;
	ctx->object = nullptr;

	iarray_t *next = c->next;

	if(next->value == TP_NULL){
		if(!(ctx->object = pedar_object_malloc(sizeof(double)))){
			printf("unable to alloc memory!\n");
			exit(-1);
		}
		ctx->object->type = TP_NULL;
		ctx->object->ptr = 0;
		return next->next;
	}
	else if(next->value == TP_IMM){
		if(!(ctx->object = pedar_object_malloc(sizeof(double)))){
			printf("unable to alloc memory!\n");
			exit(-1);
		}
		ctx->object->type = TP_NUMBER;
		ctx->object->num = (value_t)c->value;
		return next->next;
	}
	else if(next->value == TP_NUMBER){
		if(!(ctx->object = pedar_object_malloc(sizeof(double)))){
			printf("unable to alloc memory!\n");
			exit(-1);
		}
		ctx->object->type = TP_NUMBER;
		ctx->object->num = utils_atof((char *)c->value);
		return next->next;
	}
	else if(next->value == TP_CHAR){
		if(!(ctx->object = pedar_object_malloc(sizeof(char)))){
			printf("unable to alloc memory!\n");
			exit(-1);
		}
		ctx->object->type = TP_CHAR;
		ctx->object->num = (char)c->value;
		return next->next;
	}
	else if(next->value == TP_DATA){
		if(!(ctx->object = pedar_object_malloc(sizeof(void *)))){
			printf("unable to alloc memory!\n");
			exit(-1);
		}
		ctx->object->type = TP_DATA;
		ctx->object->ptr = data_from ((char *)c->value);
		return next->next;
	}

	printf("IMM, unknown type!\n");
	exit(-1);
}

iarray_t *
decode_data(context_t *ctx, iarray_t *c) {
	c = c->next;
	value_t cnt = c->value;

	variable_t *ebx;
	ebx = (variable_t *)table_content(table_rpop(ctx->table_be));

	if(ebx && ebx->value->ptr){
		// call subroutine
		if(ebx->value->type == TP_DATA){
			object_t *esp;
			esp = frame_pop(ctx->frame);

			itable_t *rp;
			if(!(rp = table_at((table_t *)ebx->value->ptr, esp->num))){
				printf("out of range [%lld]!\n", (value_t)esp->num);
				exit(-1);
			}

			if(!(rp->value = rp->value ? rp->value : pedar_object_malloc(sizeof(double)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}

			ctx->object = (object_t *)rp->value;

			return c->next;
		}
		else if(ebx->value->type == TP_CLASS){
			class_t *class = (class_t *)ebx->value->ptr;
			return fn_prepare(ctx, c, class, class->key, cnt, FN_BRACKET);
		}

		printf("call %s[], not define for this variable!\n", ebx->key);
		exit(-1);
	}

	table_t *tbl = table_create();

	value_t i;
	for(i = 0; i < cnt; i++){
		table_lpush(tbl, (value_p)frame_pop(ctx->frame));
	}

	if(!(ctx->object = pedar_object_malloc(sizeof(void *)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}

	ctx->object->type = TP_DATA;
	ctx->object->ptr = tbl;

	return c->next;
}

iarray_t *
decode_sd(context_t *ctx, iarray_t *c) {
	object_t *esp;
	// save data to address, value in object, address on table_frame
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("save data, bad pop data!\n");
		exit(-1);
	}

	switch (ctx->object->type) {
		case TP_NULL:
			if(esp->type != ctx->object->type){
				esp->ptr = pedar_object_realloc(esp->ptr, sizeof(void *));;
			}
			esp->ptr = ctx->object->ptr;
			break;
		case TP_CHAR:
			if(esp->type != ctx->object->type){
				esp->ptr = pedar_object_realloc(esp->ptr, sizeof(char));
			}
			esp->num = ctx->object->num;
			break;
		case TP_NUMBER:
			if(esp->type != ctx->object->type){
				esp->ptr = pedar_object_realloc(esp->ptr, sizeof(double));
			}
			esp->num = ctx->object->num;
			break;
		case TP_DATA:
			if(esp->type != ctx->object->type){
				esp->ptr = pedar_object_realloc(esp->ptr, sizeof(void *));;
			}
			esp->ptr = data_clone((table_t *)ctx->object->ptr);
			break;
		case TP_CLASS:
			if(esp->type != ctx->object->type){
				esp->ptr = pedar_object_realloc(esp->ptr, sizeof(void *));;
			}
			esp->ptr = (class_t *)ctx->object->ptr;
			break;
		default:
			printf("SD, unknown type!\n");
			exit(-1);
	}

	esp->type = ctx->object->type;
	ctx->object = esp;

	return c->next;
}

iarray_t *
decode_ld(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("load data, bad pop data!\n");
		exit(-1);
	}

	if(ctx->object->level == LEVEL_REFRENCE){
		esp->level = LEVEL_REFRENCE;
		ctx->variable->value = esp;
		object_delete(ctx->object);
		ctx->object = esp;
		return c->next;
	}

	switch (esp->type) {
		case TP_NULL:
			if(esp->type != ctx->object->type){
				ctx->object->ptr = pedar_object_realloc(ctx->object->ptr, sizeof(void *));
			}
			ctx->object->ptr = esp->ptr;
			break;
		case TP_CHAR:
			if(esp->type != ctx->object->type){
				ctx->object->ptr = pedar_object_realloc(ctx->object->ptr, sizeof(char));
			}
			ctx->object->num = esp->num;
			break;
		case TP_NUMBER:
			if(esp->type != ctx->object->type){
				ctx->object->ptr = pedar_object_realloc(ctx->object->ptr, sizeof(double));
			}
			ctx->object->num = esp->num;
			break;
		case TP_DATA:
			if(esp->type != ctx->object->type){
				ctx->object->ptr = pedar_object_realloc(ctx->object->ptr, sizeof(void *));
			}
			ctx->object->ptr = esp->ptr;
			break;
		case TP_CLASS:
			if(esp->type != ctx->object->type){
				ctx->object->ptr = pedar_object_realloc(ctx->object->ptr, sizeof(void *));
			}
			ctx->object->ptr = (class_t *)esp->ptr;
			break;
		default:
			printf("LD, unknown type!\n");
			exit(-1);
	}

	ctx->object->type = esp->type;

	return c->next;
}

iarray_t *
decode_push(context_t *ctx, iarray_t *c) {
	// push the value of object onto the table_frame
	frame_push(ctx->frame, ctx->object);
	ctx->object = nullptr;
	return c->next;
}

iarray_t *
decode_jmp(context_t *ctx, iarray_t *c) {
	// jump to the address
	c = c->next;
	return (iarray_t *)c->value;
}

iarray_t *
decode_jz(context_t *ctx, iarray_t *c) {
	// jump if object is zero
	c = c->next;
	return (ctx->object->num) ? c->next : (iarray_t *)c->value;
}

iarray_t *
decode_jnz(context_t *ctx, iarray_t *c) {
	// jump if object is not zero
	c = c->next;
	return (ctx->object->num) ? (iarray_t *)c->value : c->next;
}

iarray_t *
decode_cent(context_t *ctx, iarray_t *c) {
	variable_t *var;
	if(!(var = malloc(sizeof(variable_t)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}

	var->key = ctx->class->key;
	if(!(var->value = pedar_object_malloc(sizeof(void *)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}
	var->value->type = TP_CLASS;
	var->value->ptr = ctx->class;

	table_rpush(ctx->class->variables, (value_p)var);

	if(!(var = malloc(sizeof(variable_t)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}

	var->key = ctx->class->super->key;
	if(!(var->value = pedar_object_malloc(sizeof(void *)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}
	var->value->type = TP_CLASS;
	var->value->ptr = ctx->class->super;

	table_rpush(ctx->class->variables, (value_p)var);

	ctx->class->type = CLASS_BURN;

	return c->next;
}

iarray_t *
decode_clev(context_t *ctx, iarray_t *c) {
	ctx->fn = (function_t *)table_content(table_rpop(ctx->table_fn));
	ctx->class = (class_t *)table_content(table_rpop(ctx->table_class));
	ctx->frame = (frame_t *)table_content(table_rpop(ctx->table_frame));
	if(!ctx->frame){
		printf("point class leave, free frame!\n");
		exit(-1);
	}
	object_t *esp;
	esp = (object_t *)frame_pop(ctx->frame);
	return (iarray_t *)esp->ptr;
}

iarray_t *
decode_call(context_t *ctx, iarray_t *c) {
	// call subroutine
	object_t *esp;
	esp = (object_t *)frame_pop(ctx->frame);

	variable_t *var;
	var = (variable_t *)table_content(table_rpop(ctx->table_ne));

	ctx->floating = (class_t *)table_content(table_rpop(ctx->table_floating));

	return fn_prepare(ctx, c, ctx->floating ? ctx->floating : ctx->class, var->key, esp->num, FN_PAREN);
}

iarray_t *
decode_ent(context_t *ctx, iarray_t *c) {
	table_rpush(ctx->table_class, (value_p)ctx->class);
	ctx->class = ctx->fn->super;
	return c->next;
}

iarray_t *
decode_lev(context_t *ctx, iarray_t *c) {
	if(ctx->fn){
		if(strncmp(ctx->class->key, ctx->fn->key, max(strlen(ctx->class->key), strlen(ctx->fn->key))) == 0){
			if(!(ctx->variable = variable_get(ctx->class->variables, ctx->class->key))){
				printf("'this' object not defined in class %s.\n", ctx->class->key);
				exit(-1);
			}
			ctx->object = ctx->variable->value;
		}
	}

	ctx->fn = (function_t *)table_content(table_rpop(ctx->table_fn));

	ctx->class = (class_t *)table_content(table_rpop(ctx->table_class));

	ctx->frame = (frame_t *)table_content(table_rpop(ctx->table_frame));

	if(!ctx->frame){
		printf("breakpoint function leave, free frame!\n");
		exit(-1);
	}

	object_t *esp;
	esp = (object_t *)frame_pop(ctx->frame);
	return (iarray_t *)esp->ptr;
}

iarray_t *
decode_this(context_t *ctx, iarray_t *c) {
	if(!(ctx->variable = variable_get(ctx->class->variables, ctx->class->key))){
		printf("'this' object not defined in class %s.\n", ctx->class->key);
		exit(-1);
	}
	ctx->object = ctx->variable->value;
	return c->next;
}

iarray_t *
decode_super(context_t *ctx, iarray_t *c) {
	if(!(ctx->variable = variable_get(ctx->class->variables, ctx->class->super->key))){
		printf("'super' object not defined.\n");
		exit(-1);
	}
	ctx->object = ctx->variable->value;
	return c->next;
}

iarray_t *
decode_chg(context_t *ctx, iarray_t *c) {
	if(ctx->object->type == TP_CLASS){
		if((ctx->floating = (class_t *)ctx->object->ptr)){
			table_rpush(ctx->table_floating, (value_p)ctx->floating);
			if(ctx->floating->type == CLASS_RAW){
				frame_t *frame2 = frame_create();

				table_rpush(ctx->table_class, (value_p)ctx->class);
				ctx->class = ctx->floating;

				table_rpush(ctx->table_fn, (value_p)ctx->fn);
				ctx->fn = nullptr;

				object_t *esp;
				if(!(esp = pedar_object_malloc(sizeof(double)))){
					printf("unable to alloc memory!\n");
					exit(-1);
				}
				esp->type = TP_ADRS;
				esp->ptr = (value_p)c->next;

				frame_push(ctx->frame, esp);
				table_rpush(ctx->table_frame, (value_p)ctx->frame);
				ctx->frame = frame2;

				return (iarray_t *)ctx->class->start;
			}
		}
	}
	return c->next;
}

iarray_t *
decode_sizeof(context_t *ctx, iarray_t *c) {
	object_t *obj;
	switch (ctx->object->type) {
		case TP_NULL:
			if(!(obj = pedar_object_malloc(sizeof(double)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			obj->type = TP_NUMBER;
			obj->num = object_sizeof(ctx->object);
			ctx->object = obj;
			return c->next;
			break;
		case TP_CHAR:
			if(!(obj = pedar_object_malloc(sizeof(double)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			obj->type = TP_NUMBER;
			obj->num = object_sizeof(ctx->object);
			ctx->object = obj;
			return c->next;
			break;
		case TP_NUMBER:
			if(!(obj = pedar_object_malloc(sizeof(double)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			obj->type = TP_NUMBER;
			obj->num = object_sizeof(ctx->object);
			ctx->object = obj;
			return c->next;
			break;
		case TP_DATA:
			if(!(obj = pedar_object_malloc(sizeof(double)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			obj->type = TP_NUMBER;
			obj->num = data_sizeof((table_t *)ctx->object->ptr);
			ctx->object = obj;
			return c->next;
			break;
		case TP_CLASS:
			frame_push(ctx->frame, ctx->object);
			return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, "sizeof", 1, FN_PAREN);
			break;
		default:
			printf("sizeof, unknown type!\n");
			exit(-1);
			break;
	}
}

iarray_t *
decode_typeof(context_t *ctx, iarray_t *c) {
	object_t *obj;
	switch (ctx->object->type) {
		case TP_NULL:
			if(!(obj = pedar_object_malloc(sizeof(double)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			obj->type = TP_DATA;
			obj->ptr = data_from(STR_NULL);
			ctx->object = obj;
			return c->next;
			break;
		case TP_CHAR:
			if(!(obj = pedar_object_malloc(sizeof(double)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			obj->type = TP_DATA;
			obj->ptr = data_from(STR_CHAR);
			ctx->object = obj;
			return c->next;
			break;
		case TP_NUMBER:
			if(!(obj = pedar_object_malloc(sizeof(double)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			obj->type = TP_DATA;
			obj->ptr = data_from(STR_NUMBER);
			ctx->object = obj;
			return c->next;
			break;
		case TP_DATA:
			if(!(obj = pedar_object_malloc(sizeof(double)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			obj->type = TP_DATA;
			obj->ptr = data_from(STR_DATA);
			ctx->object = obj;
			return c->next;
			break;
		case TP_CLASS:
			if(!(obj = pedar_object_malloc(sizeof(double)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			obj->type = TP_DATA;
			class_t *class = ctx->object->ptr;
			obj->ptr = data_from(class->key);
			ctx->object = obj;
			return c->next;
			break;
		default:
			printf("typeof, unknown type!\n");
			exit(-1);
			break;
	}
}

iarray_t *
decode_delete(context_t *ctx, iarray_t *c) {
	object_t *esp;
	esp = frame_pop(ctx->frame);

	if(esp->type != TP_NUMBER){
		printf("delete, parameters!\n");
		exit(-1);
	}

	table_t *frame2 = table_create();
	long64_t i, cnt = esp->num;
	for(i = 0; i < cnt; i++){
		table_rpush(frame2, (value_p)frame_pop(ctx->frame));
	}

	esp = frame_pop(ctx->frame);

	if(esp->type == TP_DATA){
		table_t *tbl = (table_t *)esp->ptr;
		table_sort(frame2, object_sort);
		i = -1;
	    while((esp = (object_t *)table_content(table_lpop(frame2)))){
			if((esp->type != TP_NUMBER) || (i == esp->num) || (esp->num < 0)){
				object_delete(esp);
				continue;
			}
			i = esp->num;
	        itable_t *t = table_at(tbl, i);
	        if(t && t->value){
	            object_delete((object_t *)t->value);
	        }
	        table_unlink(tbl, t);
	        object_delete(esp);
	    }
	}
	else {
	    object_delete(esp);
	    while((esp = (object_t *)table_content(table_rpop(frame2)))){
	        object_delete(esp);
	    }
	}

	return c->next;
}

iarray_t *
decode_insert(context_t *ctx, iarray_t *c) {
	table_t *frame2 = table_create();
	object_t *esp;
	esp = (object_t *)frame_pop(ctx->frame);

	if(esp->type != TP_NUMBER){
		printf("insert, parameters!\n");
		exit(-1);
	}

	value_t i, cnt = esp->num;
	for(i = 0; i < cnt; i++){
		table_rpush(frame2, (value_p)frame_pop(ctx->frame));
	}

	esp = (object_t *)frame_pop(ctx->frame);

	if(esp->type != TP_DATA){
		printf("insert, first paramater should be a array data!\n");
		exit(-1);
	}

	table_t *tbl = (table_t *)esp->ptr;

	esp = (object_t *)table_content(table_rpop(frame2));

	if(esp->type != TP_NUMBER){
		printf("insert, two paramater should be index of array!\n");
		exit(-1);
	}

	i = esp->num;
	i = (i < 0) ? 0 : i;

	itable_t *t = table_at(tbl, i);

	while((esp = (object_t *)table_content(table_rpop(frame2)))){
		t = table_insert(tbl, t, (value_p)esp);
	}

	return c->next;
}

iarray_t *
decode_count(context_t *ctx, iarray_t *c) {
	object_t *obj;
	switch (ctx->object->type) {
		case TP_NULL:
			if(!(obj = pedar_object_malloc(sizeof(double)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			obj->type = TP_NUMBER;
			obj->num = 1;
			ctx->object = obj;
			return c->next;
			break;
		case TP_CHAR:
			if(!(obj = pedar_object_malloc(sizeof(double)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			obj->type = TP_NUMBER;
			obj->num = 1;
			ctx->object = obj;
			return c->next;
		break;
		case TP_NUMBER:
			if(!(obj = pedar_object_malloc(sizeof(double)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			obj->type = TP_NUMBER;
			obj->num = 1;
			ctx->object = obj;
			return c->next;
		break;
		case TP_DATA:
			if(!(obj = pedar_object_malloc(sizeof(double)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			obj->type = TP_NUMBER;
			table_t *tbl = (table_t *)ctx->object->ptr;
			obj->num = table_count(tbl);
			ctx->object = obj;
			return c->next;
		break;
		case TP_CLASS:
			frame_push(ctx->frame, ctx->object);
			return fn_prepare(ctx, c, (class_t *)ctx->object->ptr, "count", 1, FN_PAREN);
			break;
		default:
			printf("count, unknown type!\n");
			exit(-1);
			break;
	}
}

iarray_t *
decode_break(context_t *ctx, iarray_t *c) {
	iarray_t *b = c->next;
	while(b != ctx->code->begin){
		if(b->value == LOPE){
			c = b;
			break;
		}
		b = b->next;
	}
	return c->next;
}

iarray_t *
decode_new(context_t *ctx, iarray_t *c) {
	if(ctx->object->type == TP_CLASS){
		if((ctx->floating = (class_t *)ctx->object->ptr)){
			frame_t *frame2 = frame_create();

			table_rpush(ctx->table_class, (value_p)ctx->class);
			ctx->class = (ctx->floating->type == CLASS_RAW) ? ctx->floating : class_clone(ctx->floating);
			table_rpush(ctx->table_floating, (value_p)ctx->class);

			table_rpush(ctx->table_fn, (value_p)ctx->fn);
			ctx->fn = nullptr;

			object_t *esp;
			if(!(esp = pedar_object_malloc(sizeof(double)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			esp->type = TP_ADRS;
			esp->ptr = (value_p)c->next;

			frame_push(ctx->frame, esp);
			table_rpush(ctx->table_frame, (value_p)ctx->frame);
			ctx->frame = frame2;

			return (iarray_t *)ctx->class->start;
		}
	}
	return c->next;
}

iarray_t *
decode_ref(context_t *ctx, iarray_t *c) {
	ctx->variable->value->level = LEVEL_REFRENCE;
	return c->next;
}

iarray_t *
decode_ret(context_t *ctx, iarray_t *c) {
	if(ctx->object){
		ctx->object->level = LEVEL_RESPONSE;
	}
	return c->next;
}



iarray_t *
decode_rst(context_t *ctx, iarray_t *c) {
	ctx->floating = nullptr;
	return c->next;
}

iarray_t *
decode_spa(context_t *ctx, iarray_t *c) {
	table_rpush(ctx->table_ne, (value_p)ctx->variable);
	return c->next;
}

iarray_t *
decode_sbr(context_t *ctx, iarray_t *c) {
	table_rpush(ctx->table_be, (value_p)ctx->variable);
	return c->next;
}

iarray_t *
decode_print(context_t *ctx, iarray_t *c) {
	object_t *esp;

	table_t *parameters = table_create();

	esp = (object_t *)frame_pop(ctx->frame);

	value_t i, cnt = esp->num;

	for(i = 0; i < cnt; i++){
		table_rpush(parameters, (value_p)frame_pop(ctx->frame));
	}

	while((esp = (object_t *)table_content(table_rpop(parameters)))){
		if(esp->type == TP_NULL){
			printf("%s ", STR_NULL);
			continue;
		}
		else if(esp->type == TP_DATA){
			table_t *formated;
			if(i > 0){
				formated = table_create();
				data_format((table_t *)esp->ptr, parameters, formated);
			}
			else {
				formated = (table_t *)esp->ptr;
			}

			itable_t *t;
			while((t = table_lpop(formated))){
				object_t *obj = (object_t *)t->value;
				if(obj->type != TP_CHAR){
					printf("formated string must created of chars!\n");
					exit(-1);
				}

				if(obj->num == '\\'){
					object_delete(obj);

					t = table_lpop(formated);
					obj = (object_t *)t->value;

					if(obj->num == 'n'){
						printf("\n");
						continue;
					}
					else
					if(obj->num == 't'){
						printf("\t");
						continue;
					}
					else
					if(obj->num == 'v'){
						printf("\v");
						continue;
					}
					else
					if(obj->num == 'a'){
						printf("\a");
						continue;
					}
					else
					if(obj->num == 'b'){
						printf("\b");
						continue;
					}

					printf("\\");
					continue;
				}

				printf("%c", (char)obj->num);
				object_delete(obj);
			}

			continue;
		}
		else if(esp->type == TP_CHAR){
			printf("%c", (char)esp->num);
			continue;
		}
		else if(esp->type == TP_NUMBER){
			if((esp->num - (value_t)esp->num) != 0){
				printf("%.16f", esp->num);
			}else{
				printf("%lld", (value_t)esp->num);
			}
			continue;
		}
		else if(esp->type == TP_CLASS){
			class_t *cls = (class_t *)esp->ptr;
			printf("%s", cls->key);
			continue;
		}
		else {
			printf("%.16f", esp->num);
		}
	}

	return c->next;
}

iarray_t *
decode_format(context_t *ctx, iarray_t *c) {
	table_t *parameters = table_create();

	object_t *esp;
	esp = (object_t *)frame_pop(ctx->frame);

	value_t i, cnt = esp->num;

	for(i = 0; i < cnt; i++){
		table_rpush(parameters, (value_p)frame_pop(ctx->frame));
	}

	table_t *data = table_create();

	while((esp = (object_t *)table_content(table_rpop(parameters)))){
		if(esp->type != TP_DATA){
			table_rpush(data, (value_p)esp);
			continue;
		}

		table_t *formated = table_create();
		data_format((table_t *)esp->ptr, parameters, formated);

		if(!(ctx->object = pedar_object_malloc(sizeof(void *)))){
			printf("unable to alloc memory!\n");
			exit(-1);
		}
		ctx->object->type = TP_DATA;
		ctx->object->ptr = formated;

		table_rpush(data, (value_p)ctx->object);
	}

	if(!(ctx->object = pedar_object_malloc(sizeof(void *)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}
	ctx->object->type = TP_DATA;
	ctx->object->ptr = data;

	return c->next;
}

iarray_t *
decode_open(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("open, bad pop object!\n");
		exit(-1);
	}

	if(esp->type != TP_DATA){
		printf("open, path must be a string!\n");
		exit(-1);
	}

	char *path = data_to ((table_t *)esp->ptr);
	path[table_count((table_t *)esp->ptr)] = '\0';

	if(ctx->object->type != TP_NUMBER){
		printf("open, object not a number!\n");
		exit(-1);
	}

	// O_WRONLY | O_RDWR | O_RDONLY | O_CREAT
	int flag = (int)ctx->object->num;

	object_t *obj;
	if(!(obj = pedar_object_malloc(sizeof(double)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}

	obj->type = TP_NUMBER;
	obj->num = open(path, flag);

	pedar_free(path);

	object_delete (esp);
	object_delete (ctx->object);

	ctx->object = obj;

	return c->next;
}

iarray_t *
decode_close(context_t *ctx, iarray_t *c) {
	if(ctx->object->type != TP_NUMBER){
		printf("close, object not a number!\n");
		exit(-1);
	}

	object_t *obj;
	if(!(obj = pedar_object_malloc(sizeof(double)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}
	obj->type = TP_NUMBER;
	obj->num = close(ctx->object->num);

	object_delete (ctx->object);

	ctx->object = obj;

	return c->next;
}

iarray_t *
decode_read(context_t *ctx, iarray_t *c) {
	if(ctx->object->type != TP_NUMBER){
		printf("read, object not a number!\n");
		exit(-1);
	}

	char *buf = pedar_malloc(ctx->object->num * sizeof(char));

	object_t *obj_buf;
	if(!(obj_buf = (object_t *)frame_pop(ctx->frame))){
		printf("read, buffer is null!\n");
		exit(-1);
	}
	obj_buf->type = TP_DATA;

	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("read, buffer is null!\n");
		exit(-1);
	}

	object_t *obj;
	if(!(obj = pedar_object_malloc(sizeof(double)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}
	obj->type = TP_NUMBER;
	obj->num = read(esp->num, buf, ctx->object->num);

	obj_buf->ptr = data_from (buf);
	pedar_free(buf);

	object_delete (ctx->object);

	ctx->object = obj;

	return c->next;
}

iarray_t *
decode_write(context_t *ctx, iarray_t *c) {
	object_t *esp;

	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("write, buffer is null!\n");
		exit(-1);
	}

	char *buf = data_to ((table_t *)esp->ptr);

	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("write, buffer is null!\n");
		exit(-1);
	}

	if(ctx->object->type != TP_NUMBER){
		printf("write, object not a number!\n");
		exit(-1);
	}

	object_t *obj;
	if(!(obj = pedar_object_malloc(sizeof(double)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}
	obj->type = TP_NUMBER;
	obj->num = write(esp->num, buf, ctx->object->num);

	pedar_free(buf);

	object_delete (ctx->object);
	object_delete (esp);

	ctx->object = obj;

	return c->next;
}

iarray_t *
decode_seek(context_t *ctx, iarray_t *c) {
	object_t *esp;

	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("seek, buffer is null!\n");
		exit(-1);
	}

	long n = esp->num;

	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("seek, buffer is null!\n");
		exit(-1);
	}

	if(ctx->object->type != TP_NUMBER){
		printf("seek, object not a number!\n");
		exit(-1);
	}

	object_t *obj;
	if(!(obj = pedar_object_malloc(sizeof(double)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}
	obj->type = TP_NUMBER;
	obj->num = lseek(esp->num, n, ctx->object->num);

	object_delete (ctx->object);
	object_delete (esp);

	ctx->object = obj;

	return c->next;
}

iarray_t *
decode_rename(context_t *ctx, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)frame_pop(ctx->frame))){
		printf("rename, bad pop object!\n");
		exit(-1);
	}

	if(esp->type != TP_DATA){
		printf("rename, path must be a string!\n");
		exit(-1);
	}

	char *path = data_to ((table_t *)esp->ptr);
	path[table_count((table_t *)esp->ptr)] = '\0';

	if(ctx->object->type != TP_DATA){
		printf("rename, object not a string!\n");
		exit(-1);
	}

	char *path2 = data_to ((table_t *)ctx->object->ptr);
	path2[table_count((table_t *)ctx->object->ptr)] = '\0';

	object_t *obj;
	if(!(obj = pedar_object_malloc(sizeof(double)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}

	obj->type = TP_NUMBER;
	obj->num = rename(path, path2);

	pedar_free(path);
	pedar_free(path2);

	object_delete (esp);
	object_delete (ctx->object);

	ctx->object = obj;

	return c->next;
}

iarray_t *
decode_cwd(context_t *ctx, iarray_t *c) {
	object_t *obj;
	char cwd[MAX_PATH];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		if(!(obj = pedar_object_malloc(sizeof(void *)))){
			printf("unable to alloc memory!\n");
			exit(-1);
		}
		obj->type = TP_DATA;
		obj->ptr = data_from(cwd);
	}else{
		if(!(obj = pedar_object_malloc(sizeof(void *)))){
			printf("unable to alloc memory!\n");
			exit(-1);
		}
		obj->type = TP_DATA;
		obj->ptr = data_from(STR_NULL);
	}

	ctx->object = obj;

	return c->next;
}

iarray_t *
decode_chdir(context_t *ctx, iarray_t *c) {
	if(ctx->object->type != TP_DATA){
		printf("chdir, object not a string!\n");
		exit(-1);
	}

	char *path = data_to ((table_t *)ctx->object->ptr);

	object_t *obj;
	if(!(obj = pedar_object_malloc(sizeof(double)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}

	obj->type = TP_NUMBER;
	obj->num = chdir(path);

	pedar_free(path);

	object_delete (ctx->object);

	ctx->object = obj;

	return c->next;
}

iarray_t *
decode_walk(context_t *ctx, iarray_t *c) {
	object_t *obj;

	if(ctx->object->type != TP_DATA){
		printf("walk, object not a string!\n");
		exit(-1);
	}

	char *path = data_to ((table_t *)ctx->object->ptr);
	path[table_count((table_t *)ctx->object->ptr)] = '\0';

	table_t *tbl = table_create();

	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir (path)) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			if(!(obj = pedar_object_malloc(sizeof(void *)))){
				printf("unable to alloc memory!\n");
				exit(-1);
			}
			obj->type = TP_DATA;
			obj->ptr = data_from(ent->d_name);
			table_rpush(tbl, (value_p)obj);
		}
		closedir (dir);
	} else {
	  /* could not open directory */
	  tbl = nullptr;
	}

	if(!(obj = pedar_object_malloc(sizeof(void *)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}
	obj->type = TP_DATA;
	obj->ptr = tbl;

	ctx->object = obj;

	return c->next;
}

iarray_t *
decode_gets(context_t *ctx, iarray_t *c) {
	object_t *obj;

	if(!(obj = pedar_object_malloc(sizeof(void *)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}

	if(ctx->object->type != TP_NUMBER){
		printf("gets, object not a string!\n");
		exit(-1);
	}

	char *str = pedar_malloc(sizeof(char) * (value_t)ctx->object->num);
	fflush(stdin);

	obj->type = TP_DATA;
	obj->ptr = data_from(fgets(str,MAX_PATH,stdin));

	ctx->object = obj;

	return c->next;
}

iarray_t *
decode_getkey(context_t *ctx, iarray_t *c) {
	object_t *obj;

	if(!(obj = pedar_object_malloc(sizeof(void *)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}

	char str[2];

	str[0] = getch();
	str[1] = '\0';

	obj->type = TP_DATA;
	obj->ptr = data_from(str);

	ctx->object = obj;

	return c->next;
}

iarray_t *
decode_tick(context_t *ctx, iarray_t *c) {
	object_t *obj;

	if(!(obj = pedar_object_malloc(sizeof(double)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}
	obj->type = TP_NUMBER;
	obj->num = (value_t)clock();

	ctx->object = obj;

	return c->next;
}

iarray_t *
decode_time(context_t *ctx, iarray_t *c) {
	object_t *obj;

	if(!(obj = pedar_object_malloc(sizeof(double)))){
		printf("unable to alloc memory!\n");
		exit(-1);
	}
	obj->type = TP_NUMBER;
	obj->num = (value_t)time(NULL);

	ctx->object = obj;

	return c->next;
}

iarray_t *
decode_setup(context_t *ctx, iarray_t *c) {
	c = c->next;

	if(!(ctx->class = class_get(ctx->base, (char*)c->value))){
		printf("eval: in new operator class %s not definition!\n", (char *)c->value);
		exit(-1);
	}

	ctx->frame = frame_create();

	ctx->floating = nullptr;
	return (iarray_t *)ctx->class->start;
}

iarray_t *
decode(context_t *ctx, iarray_t *c) {
	switch (c->value) {
		case NUL:
			return c->next;
			break;
		case LOPB:
			return c->next;
			break;
		case LOPE:
			return c->next;
			break;

		case OR:
			return decode_or(ctx, c);
			break;
		case LOR:
			return decode_lor(ctx, c);
			break;
		case XOR:
			return decode_xor(ctx, c);
			break;
		case AND:
			return decode_and(ctx, c);
			break;
		case LAND:
			return decode_land(ctx, c);
			break;
		case EQ:
			return decode_eq(ctx, c);
			break;
		case NE:
			return decode_ne(ctx, c);
			break;
		case LT:
			return decode_lt(ctx, c);
			break;
		case LE:
			return decode_le(ctx, c);
			break;
		case GT:
			return decode_gt(ctx, c);
			break;
		case GE:
			return decode_ge(ctx, c);
			break;
		case LSHIFT:
			return decode_lshift(ctx, c);
			break;
		case RSHIFT:
			return decode_rshift(ctx, c);
			break;
		case ADD:
			return decode_add(ctx, c);
			break;
		case SUB:
			return decode_sub(ctx, c);
			break;
		case MUL:
			return decode_mul(ctx, c);
			break;
		case DIV:
			return decode_div(ctx, c);
			break;
		case MOD:
			return decode_mod(ctx, c);
			break;

			// VAR IMM DATA SD LD PUSH JMP JZ JNZ CENT CLEV CALL ENT LEV THIS SUPER CHG
			// SIZEOF TYPEOF DELETE INSERT COUNT BREAK NEW REF RET
		case VAR:
			return decode_var(ctx, c);
			break;
		case IMM:
			return decode_imm(ctx, c);
			break;
		case DATA:
			return decode_data(ctx, c);
			break;
		case SD:
			return decode_sd(ctx, c);
			break;
		case LD:
			return decode_ld(ctx, c);
			break;
		case PUSH:
			return decode_push(ctx, c);
			break;
		case JMP:
			return decode_jmp(ctx, c);
			break;
		case JZ:
			return decode_jz(ctx, c);
			break;
		case JNZ:
			return decode_jnz(ctx, c);
			break;
		case CENT:
			return decode_cent(ctx, c);
			break;
		case CLEV:
			return decode_clev(ctx, c);
			break;
		case CALL:
			return decode_call(ctx, c);
			break;
		case ENT:
			return decode_ent(ctx, c);
			break;
		case LEV:
			return decode_lev(ctx, c);
			break;
		case THIS:
			return decode_this(ctx, c);
			break;
		case SUPER:
			return decode_super(ctx, c);
			break;
		case CHG:
			return decode_chg(ctx, c);
			break;
		case SIZEOF:
			return decode_sizeof(ctx, c);
			break;
		case TYPEOF:
			return decode_typeof(ctx, c);
			break;
		case DELETE:
			return decode_delete(ctx, c);
			break;
		case INSERT:
			return decode_insert(ctx, c);
			break;
		case COUNT:
			return decode_count(ctx, c);
			break;
		case BREAK:
			return decode_break(ctx, c);
			break;
		case NEW:
			return decode_new(ctx, c);
			break;
		case REF:
			return decode_ref(ctx, c);
			break;
		case RET:
			return decode_ret(ctx, c);
			break;

		case RST:
			return decode_rst(ctx, c);
			break;
		case SPA:
			return decode_spa(ctx, c);
			break;
		case SBR:
			return decode_sbr(ctx, c);
			break;
		case PRTF:
			return decode_print(ctx, c);
			break;
		case FORMAT:
			return decode_format(ctx, c);
			break;
		case OPEN:
			return decode_open(ctx, c);
			break;
		case CLOSE:
			return decode_close(ctx, c);
			break;
		case READ:
			return decode_read(ctx, c);
			break;
		case WRITE:
			return decode_write(ctx, c);
			break;
		case SEEK:
			return decode_seek(ctx, c);
			break;
		case RENAME:
			return decode_rename(ctx, c);
			break;
		case CWD:
			return decode_cwd(ctx, c);
			break;
		case CHDIR:
			return decode_chdir(ctx, c);
			break;
		case WALK:
			return decode_walk(ctx, c);
			break;
		case GETKEY:
			return decode_getkey(ctx, c);
			break;
		case GETS:
			return decode_gets(ctx, c);
			break;
		case TICK:
			return decode_tick(ctx, c);
			break;
		case TIME:
			return decode_time(ctx, c);
			break;
		case SETUP:
			return decode_setup(ctx, c);
			break;
		case EXIT:
			return ctx->code->end;
			break;

		default:
			printf("unknown instruction\n");
            exit(-1);
	}

	return c;
}

void
eval(class_t *base, array_t *code)
{
    variable_t *variable = nullptr;

    /* floating save refrence of current class */
    class_t *floating = nullptr;

    object_t *object = nullptr;

	table_t *table_be = table_create();
	table_t *table_floating = table_create();
	table_t *table_ne = table_create();

    /* class save refrence of parrent class */
    class_t *class = nullptr;
    table_t *table_class = table_create();

    /* fn save refrence of path func in variables */
    function_t *fn = nullptr;
    table_t *table_fn = table_create();

    //table_t *frame = nullptr;
	frame_t *frame = nullptr;
    table_t *table_frame = table_create();

	context_t *ctx = pedar_malloc(sizeof(context_t));
	ctx = context_update(
		ctx,
		class,
		table_class,
		fn,
		table_fn,
		floating,
		table_floating,
		frame,
		table_frame,
		table_be,
		table_ne,
		variable,
		object,
		base,
		code
	);

	iarray_t *c = code->begin;
	while(c != code->end){
		//printf("decode %lld\n", c->value);
		c = decode(ctx, c);
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

    value_t i;
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
    value_t pos = ftell(fd);
    // Go to end
    fseek(fd, 0, SEEK_END);
    // read the position which is the size
    value_t chunk = ftell(fd);
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
    pedar_free(buf);

    class_t *base;
    array_t *code = array_create();
    base = parser(tokens, argc, argv, code);

    eval(base, code);

    table_destroy(tokens, token_destroy);
    return 0;
}
