#pragma once

class_t *
library_file(class_t *clspar, array_t *code){

    function_t *fun;
    iarray_t *fn_jmp, *cls_jmp;
    iarray_t *a, *b;
    class_t *clscur;

    clscur = (class_t *)malloc(sizeof(class_t));
    clscur->key = "file";
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

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"fd");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_IMM);

    array_rpush(code, SD);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"mode");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, (value_t)"r");
    array_rpush(code, TP_DATA);

    array_rpush(code, SD);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"path");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_NULL);

    array_rpush(code, SD);

    /*
    this(str, md){
        if(typeof(md) != 'DATA'){
            print('error, mode is a string variable\nr:readable, w:writable, rw:readable and writable, c:create and open\n');
            return null;
        }

        mode = md;

        open_mode = 0;
        if(mode == 'w'){
            open_mode = 1;
        }
        else if(mode == 'rw'){
            open_mode = 2;
        }
        else if(mode == 'c'){
            open_mode = 64;
        }

        fd = __open__(str, open_mode);
        path = str;
        return this;
    }
    */

    fun = (function_t *)malloc(sizeof(function_t));
    fun->key = "open";
    fun->type = FN_PAREN;
    fun->n = 2;
    fun->variables = table_create();
    fun->super = clscur;

    table_rpush(clscur->functions, (value_p)fun);

    array_rpush(code, JMP);
    fn_jmp = array_rpush(code, 0);

    fun->start = array_rpush(code, ENT);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"file");

    array_rpush(code, SVPA);
    array_rpush(code, NEW);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"str");

    array_rpush(code, LD);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"md");

    array_rpush(code, LD);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"md");

    array_rpush(code, TYPEOF);

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, (value_t)"DATA");
    array_rpush(code, TP_DATA);

    array_rpush(code, NE);

    array_rpush(code, JZ);
    a = array_rpush(code, 0);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_NULL);

    array_rpush(code, RET);
    array_rpush(code, LEV);

    a->value = (long64_t) array_rpush(code, NUL);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"mode");

    array_rpush(code, PUSH);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"md");

    array_rpush(code, SD);

    // open_mode = 0;
    array_rpush(code, VAR);
    array_rpush(code, (value_t)"open_mode");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_IMM);

    array_rpush(code, SD);


    array_rpush(code, VAR);
    array_rpush(code, (value_t)"mode");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, (value_t)"w");
    array_rpush(code, TP_DATA);

    array_rpush(code, EQ);

    array_rpush(code, JZ);
    a = array_rpush(code, 0);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"open_mode");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 1);
    array_rpush(code, TP_IMM);

    array_rpush(code, SD);

    a->value = (long64_t) array_rpush(code, NUL);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"mode");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, (value_t)"rw");
    array_rpush(code, TP_DATA);

    array_rpush(code, EQ);

    array_rpush(code, JZ);
    a = array_rpush(code, 0);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"open_mode");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 2);
    array_rpush(code, TP_IMM);

    array_rpush(code, SD);

    a->value = (long64_t) array_rpush(code, NUL);


    array_rpush(code, VAR);
    array_rpush(code, (value_t)"mode");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, (value_t)"c");
    array_rpush(code, TP_DATA);

    array_rpush(code, EQ);

    array_rpush(code, JZ);
    a = array_rpush(code, 0);


    array_rpush(code, VAR);
    array_rpush(code, (value_t)"open_mode");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 64);
    array_rpush(code, TP_IMM);

    array_rpush(code, SD);

    a->value = (long64_t) array_rpush(code, NUL);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"fd");

    array_rpush(code, PUSH);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"str");

    array_rpush(code, PUSH);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"open_mode");

    array_rpush(code, OPEN);

    array_rpush(code, SD);

    // path = str;
    array_rpush(code, VAR);
    array_rpush(code, (value_t)"path");

    array_rpush(code, PUSH);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"str");

    array_rpush(code, SD);

    array_rpush(code, THIS);

    array_rpush(code, RET);
    array_rpush(code, LEV);

    fun->end = array_rpush(code, LEV);
    fn_jmp->value = (value_t)array_rpush(code, NUL);

    /*
    close(){
        if(fd > 0){
            ret = __close__(fd);
            fd = 0;
            return ret;
        }
        return -1;
    }
    */

    fun = (function_t *)malloc(sizeof(function_t));
    fun->key = "close";
    fun->type = FN_PAREN;
    fun->n = 0;
    fun->variables = table_create();
    fun->super = clscur;

    table_rpush(clscur->functions, (value_p)fun);

    array_rpush(code, JMP);
    fn_jmp = array_rpush(code, 0);

    fun->start = array_rpush(code, ENT);


    array_rpush(code, VAR);
    array_rpush(code, (value_t)"fd");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_IMM);

    array_rpush(code, GT);

    array_rpush(code, JZ);
    a = array_rpush(code, 0);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"ret");

    array_rpush(code, PUSH);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"fd");

    array_rpush(code, CLOSE);

    array_rpush(code, SD);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"fd");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_IMM);

    array_rpush(code, SD);

    a->value = (long64_t) array_rpush(code, NUL);

    array_rpush(code, IMM);
    array_rpush(code, -1);
    array_rpush(code, TP_IMM);

    array_rpush(code, RET);
    array_rpush(code, LEV);

    fun->end = array_rpush(code, LEV);
    fn_jmp->value = (value_t)array_rpush(code, NUL);


    /*
    seek(index, flag){
        if(fd > 0){
            seek_mode = 0;
            if(flag == 'c'){
                seek_mode = 1;
            }
            else if(flag == 'e'){
                seek_mode = 2;
            }
            return __seek__(fd, index, seek_mode);
        }
        return -1;
    }
    */

    fun = (function_t *)malloc(sizeof(function_t));
    fun->key = "seek";
    fun->type = FN_PAREN;
    fun->n = 2;
    fun->variables = table_create();
    fun->super = clscur;

    table_rpush(clscur->functions, (value_p)fun);

    array_rpush(code, JMP);
    fn_jmp = array_rpush(code, 0);

    fun->start = array_rpush(code, ENT);


    array_rpush(code, VAR);
    array_rpush(code, (value_t)"index");

    array_rpush(code, LD);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"flag");

    array_rpush(code, LD);

    // fd > 0
    array_rpush(code, VAR);
    array_rpush(code, (value_t)"fd");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_IMM);

    array_rpush(code, GT);

    // if
    array_rpush(code, JZ);
    a = array_rpush(code, 0);

    // seek_mode = 0;
    array_rpush(code, VAR);
    array_rpush(code, (value_t)"seek_mode");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_IMM);

    array_rpush(code, SD);

    // flag == 'c'
    array_rpush(code, VAR);
    array_rpush(code, (value_t)"flag");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, (value_t)"c");
    array_rpush(code, TP_DATA);

    array_rpush(code, EQ);

    // if
    array_rpush(code, JZ);
    b = array_rpush(code, 0);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"seek_mode");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 1);
    array_rpush(code, TP_IMM);

    array_rpush(code, SD);

    b->value = (long64_t) array_rpush(code, NUL);

    // flag == 'c'
    array_rpush(code, VAR);
    array_rpush(code, (value_t)"flag");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, (value_t)"e");
    array_rpush(code, TP_DATA);

    array_rpush(code, EQ);

    // if
    array_rpush(code, JZ);
    b = array_rpush(code, 0);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"seek_mode");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 2);
    array_rpush(code, TP_IMM);

    array_rpush(code, SD);

    b->value = (long64_t) array_rpush(code, NUL);

    // return __seek__(fd, index, seek_mode);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"fd");

    array_rpush(code, PUSH);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"index");

    array_rpush(code, PUSH);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"seek_mode");

    array_rpush(code, SEEK);

    array_rpush(code, RET);
    array_rpush(code, LEV);

    a->value = (long64_t) array_rpush(code, NUL);

    array_rpush(code, IMM);
    array_rpush(code, -1);
    array_rpush(code, TP_IMM);

    array_rpush(code, RET);
    array_rpush(code, LEV);

    fun->end = array_rpush(code, LEV);
    fn_jmp->value = (value_t)array_rpush(code, NUL);



    /*
    tell(){
        if(fd > 0){
            return __seek__(fd, 0, 1);
        }
        return -1;
    }
    */


    fun = (function_t *)malloc(sizeof(function_t));
    fun->key = "tell";
    fun->type = FN_PAREN;
    fun->n = 0;
    fun->variables = table_create();
    fun->super = clscur;

    table_rpush(clscur->functions, (value_p)fun);

    array_rpush(code, JMP);
    fn_jmp = array_rpush(code, 0);

    fun->start = array_rpush(code, ENT);

    // fd > 0
    array_rpush(code, VAR);
    array_rpush(code, (value_t)"fd");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_IMM);

    array_rpush(code, GT);

    // if
    array_rpush(code, JZ);
    a = array_rpush(code, 0);


    // return __seek__(fd, 0, 1);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"fd");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_IMM);

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 1);
    array_rpush(code, TP_IMM);

    array_rpush(code, SEEK);

    array_rpush(code, RET);
    array_rpush(code, LEV);

    a->value = (long64_t) array_rpush(code, NUL);

    array_rpush(code, IMM);
    array_rpush(code, -1);
    array_rpush(code, TP_IMM);

    array_rpush(code, RET);
    array_rpush(code, LEV);

    fun->end = array_rpush(code, LEV);
    fn_jmp->value = (value_t)array_rpush(code, NUL);


    /*
    length(){
        if(fd > 0){
            len = __seek__(fd, 0, 2);
            __seek__(fd, 0, 0);
            return len;
        }
        return -1;
    }
    */
    fun = (function_t *)malloc(sizeof(function_t));
    fun->key = "length";
    fun->type = FN_PAREN;
    fun->n = 0;
    fun->variables = table_create();
    fun->super = clscur;

    table_rpush(clscur->functions, (value_p)fun);

    array_rpush(code, JMP);
    fn_jmp = array_rpush(code, 0);

    fun->start = array_rpush(code, ENT);

    // fd > 0
    array_rpush(code, VAR);
    array_rpush(code, (value_t)"fd");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_IMM);

    array_rpush(code, GT);

    // if
    array_rpush(code, JZ);
    a = array_rpush(code, 0);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"len");

    array_rpush(code, PUSH);

    // len = __seek__(fd, 0, 2);
    array_rpush(code, VAR);
    array_rpush(code, (value_t)"fd");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_IMM);

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 2);
    array_rpush(code, TP_IMM);

    array_rpush(code, SEEK);
    array_rpush(code, SD);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"fd");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_IMM);

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_IMM);

    array_rpush(code, SEEK);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"len");

    array_rpush(code, RET);
    array_rpush(code, LEV);

    a->value = (long64_t) array_rpush(code, NUL);

    array_rpush(code, IMM);
    array_rpush(code, -1);
    array_rpush(code, TP_IMM);

    array_rpush(code, RET);
    array_rpush(code, LEV);

    fun->end = array_rpush(code, LEV);
    fn_jmp->value = (value_t)array_rpush(code, NUL);


    /*
    read(len){
        if(fd > 0){
            if(__read__(fd, buf, len) < 0){
                return null;
            }
            return buf;
        }
        return null;
    }
    */
    fun = (function_t *)malloc(sizeof(function_t));
    fun->key = "read";
    fun->type = FN_PAREN;
    fun->n = 1;
    fun->variables = table_create();
    fun->super = clscur;

    table_rpush(clscur->functions, (value_p)fun);

    array_rpush(code, JMP);
    fn_jmp = array_rpush(code, 0);

    fun->start = array_rpush(code, ENT);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"len");

    array_rpush(code, LD);

    // fd > 0
    array_rpush(code, VAR);
    array_rpush(code, (value_t)"fd");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_IMM);

    array_rpush(code, GT);

    // if
    array_rpush(code, JZ);
    a = array_rpush(code, 0);

    //__read__(fd, buf, len) < 0
    array_rpush(code, VAR);
    array_rpush(code, (value_t)"fd");

    array_rpush(code, PUSH);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"buf");

    array_rpush(code, PUSH);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"len");

    array_rpush(code, READ);

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_IMM);

    array_rpush(code, LT);

    array_rpush(code, JZ);
    b = array_rpush(code, 0);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_NULL);

    array_rpush(code, RET);
    array_rpush(code, LEV);

    b->value = (long64_t) array_rpush(code, NUL);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"buf");

    array_rpush(code, RET);
    array_rpush(code, LEV);

    a->value = (long64_t) array_rpush(code, NUL);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_NULL);;

    array_rpush(code, RET);
    array_rpush(code, LEV);

    fun->end = array_rpush(code, LEV);
    fn_jmp->value = (value_t)array_rpush(code, NUL);






    /*
    write(buf){
        if(fd > 0){
            return __write__(fd, buf, count(buf));
        }
        return -1;
    }
    */
    fun = (function_t *)malloc(sizeof(function_t));
    fun->key = "write";
    fun->type = FN_PAREN;
    fun->n = 1;
    fun->variables = table_create();
    fun->super = clscur;

    table_rpush(clscur->functions, (value_p)fun);

    array_rpush(code, JMP);
    fn_jmp = array_rpush(code, 0);

    fun->start = array_rpush(code, ENT);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"buf");

    array_rpush(code, LD);

    // fd > 0
    array_rpush(code, VAR);
    array_rpush(code, (value_t)"fd");

    array_rpush(code, PUSH);

    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_IMM);

    array_rpush(code, GT);

    // if
    array_rpush(code, JZ);
    a = array_rpush(code, 0);

    //return __write__(fd, buf, len)
    array_rpush(code, VAR);
    array_rpush(code, (value_t)"fd");

    array_rpush(code, PUSH);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"buf");

    array_rpush(code, PUSH);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"buf");

    array_rpush(code, COUNT);

    array_rpush(code, WRITE);

    array_rpush(code, RET);
    array_rpush(code, LEV);

    a->value = (long64_t) array_rpush(code, NUL);

    array_rpush(code, IMM);
    array_rpush(code, -1);
    array_rpush(code, TP_NULL);;

    array_rpush(code, RET);
    array_rpush(code, LEV);

    fun->end = array_rpush(code, LEV);
    fn_jmp->value = (value_t)array_rpush(code, NUL);


    // rename
    fun = (function_t *)malloc(sizeof(function_t));
    fun->key = "rename";
    fun->type = FN_PAREN;
    fun->n = 2;
    fun->variables = table_create();
    fun->super = clscur;

    table_rpush(clscur->functions, (value_p)fun);

    array_rpush(code, JMP);
    fn_jmp = array_rpush(code, 0);

    fun->start = array_rpush(code, ENT);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"path_src");

    array_rpush(code, LD);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"path_dst");

    array_rpush(code, LD);

    //
    array_rpush(code, VAR);
    array_rpush(code, (value_t)"path_src");

    array_rpush(code, PUSH);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"path_dst");

    array_rpush(code, RENAME);

    fun->end = array_rpush(code, LEV);
    fn_jmp->value = (value_t)array_rpush(code, NUL);

    // cwd
    fun = (function_t *)malloc(sizeof(function_t));
    fun->key = "cwd";
    fun->type = FN_PAREN;
    fun->n = 0;
    fun->variables = table_create();
    fun->super = clscur;

    table_rpush(clscur->functions, (value_p)fun);

    array_rpush(code, JMP);
    fn_jmp = array_rpush(code, 0);

    fun->start = array_rpush(code, ENT);

    array_rpush(code, CWD);

    fun->end = array_rpush(code, LEV);
    fn_jmp->value = (value_t)array_rpush(code, NUL);

    // chdir
    fun = (function_t *)malloc(sizeof(function_t));
    fun->key = "chdir";
    fun->type = FN_PAREN;
    fun->n = 1;
    fun->variables = table_create();
    fun->super = clscur;

    table_rpush(clscur->functions, (value_p)fun);

    array_rpush(code, JMP);
    fn_jmp = array_rpush(code, 0);

    fun->start = array_rpush(code, ENT);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"path_dst");

    array_rpush(code, LD);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"path_dst");

    array_rpush(code, CHDIR);

    fun->end = array_rpush(code, LEV);
    fn_jmp->value = (value_t)array_rpush(code, NUL);


    // walk
    fun = (function_t *)malloc(sizeof(function_t));
    fun->key = "walk";
    fun->type = FN_PAREN;
    fun->n = 1;
    fun->variables = table_create();
    fun->super = clscur;

    table_rpush(clscur->functions, (value_p)fun);

    array_rpush(code, JMP);
    fn_jmp = array_rpush(code, 0);

    fun->start = array_rpush(code, ENT);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"path_dst");

    array_rpush(code, LD);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"res");

    array_rpush(code, PUSH);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"path_dst");

    array_rpush(code, WALK);
    array_rpush(code, SD);

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"res");

    array_rpush(code, RET);
    array_rpush(code, LEV);

    fun->end = array_rpush(code, LEV);
    fn_jmp->value = (value_t)array_rpush(code, NUL);

    clscur->end = array_rpush(code, CLEV);
    cls_jmp->value = (value_t)array_rpush(code, NUL);

    return clscur;
}
