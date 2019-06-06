#pragma once

#include "file.h"
#include "keyboard.h"

class_t *
library_io(class_t *clspar, array_t *code){
    //function_t *fun;
    iarray_t *cls_jmp;//, *fn_jmp;
    //iarray_t *a, *b;
    class_t *clscur;

    clscur = (class_t *)malloc(sizeof(class_t));
    clscur->key = "io";
    clscur->type = CLASS_RAW;

    clscur->parents = table_create();
    clscur->childrens = table_create();
    clscur->variables = table_create();
    clscur->functions = table_create();

    clscur->super = clspar;

    table_rpush(clscur->parents, (value_p)clspar);
    table_rpush(clspar->childrens, (value_p)clscur);
    array_rpush(code, JMP);
    cls_jmp = array_rpush(code, 0);

    clscur->start = array_rpush(code, CENT);


    library_file(clscur, code);
    library_keyboard(clscur, code);

    clscur->end = array_rpush(code, CLEV);
    cls_jmp->value = (value_t)array_rpush(code, NUL);

    return clscur;
}
