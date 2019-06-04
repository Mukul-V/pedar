class_t *
library_keyboard(class_t *clspar, array_t *code){
    function_t *fun;
    iarray_t *fn_jmp, *cls_jmp;
    //iarray_t *a, *b;
    class_t *clscur;

    clscur = (class_t *)malloc(sizeof(class_t));
    clscur->key = "keyboard";
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


    // gets
    fun = (function_t *)malloc(sizeof(function_t));
    fun->key = "gets";
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

    array_rpush(code, VAR);
    array_rpush(code, (value_t)"len");

    array_rpush(code, GETS);

    fun->end = array_rpush(code, LEV);
    fn_jmp->value = (value_t)array_rpush(code, NUL);



    // get
    fun = (function_t *)malloc(sizeof(function_t));
    fun->key = "get";
    fun->type = FN_PAREN;
    fun->n = 0;
    fun->variables = table_create();
    fun->super = clscur;

    table_rpush(clscur->functions, (value_p)fun);

    array_rpush(code, JMP);
    fn_jmp = array_rpush(code, 0);

    fun->start = array_rpush(code, ENT);

    array_rpush(code, GETKEY);

    fun->end = array_rpush(code, LEV);
    fn_jmp->value = (value_t)array_rpush(code, NUL);


    clscur->end = array_rpush(code, CLEV);
    cls_jmp->value = (value_t)array_rpush(code, NUL);

    return clscur;
}
