#include "../headers/mips.h"

// print instr 1 arg
#define pins1(s1) \
    fprintf(f, "\t%s\n", s1);

// print instr 2 args
#define pins2(s1, s2) \
    fprintf(f, "\t%-5s %s\n", s1, s2);

// print instr 3 args
#define pins3(s1, s2, s3) \
    fprintf(f, "\t%-5s %-5s %s\n", s1, s2, s3);

// print instr 4 args
#define pins4(s1, s2, s3, s4) \
    fprintf(f, "\t%-5s %-5s %-5s %s\n", s1, s2, s3, s4);

// print data
#define pdat(s1, s2) \
    fprintf(f, "\t%-16s :  %s\n", s1, s2);

// print label
#define plab(s) \
    fprintf(f, "\n%s:\n", s);

// print commentary
#define pcom(s) \
    fprintf(f, "\t\t\t# %s\n", s);

// print directive
#define pdir(s) \
    fprintf(f, "%s\n", s);

// snprintf test
#define snpt(res) \
    if (res < 0 || res >= LEN) \
        ferr(__LINE__ ,"mips.c snprintf");

#define pload(reg, sym) \
    snpt(snprintf(tbuf, LEN, "%s", sym->type == S_BOOL ? "lb" : "lw")); \
    if (sym->ref) { \
        pins3(tbuf, "$t9", sym->id); \
        pins3(tbuf, reg, "0($t9)"); \
    } else \
        pins3(tbuf, reg, sym->id);

#define pstore(reg, sym) \
    snpt(snprintf(tbuf, LEN, "%s", sym->type == S_BOOL ? "sb" : "sw")); \
    if (sym->ref) { \
        pins3(tbuf, "$t9", sym->id); \
        pins3(tbuf, reg, "0($t9)"); \
    } else \
        pins3(tbuf, reg, sym->id);

static char tbuf[LEN], tbuf2[LEN], tbuf3[LEN], tbuf4[LEN];
static symbol *curfun = NULL;
static quad *curquad  = NULL;
/**
 * curfun is used to know if a funcall is done inside a function or not
 * If a funcall is done inside a function, save all usefull function local vars to the stack and restore them after the call
 */

// get debug sym ID => if sym is not ref, put it ID in buf, otherwise put "*ID" in buf
static int dsymidcnt = 0; // alternate between tbuf2, tbuf3 and tbuf4 for each call: this make possible to use the result of this fonction in max 3 args of a function call :)
char *dsymid(symbol *sym) {
    char *buf;
    if (dsymidcnt % 3 == 0) buf = tbuf2;
    else if (dsymidcnt % 3 == 1) buf = tbuf3;
    else buf = tbuf4;

    dsymidcnt ++;
    snpt(snprintf(buf, LEN, "%s%s", sym->ref ? "*" : "", sym->id));
    return buf;
}

void ferr (int line , char *s ) {
    fprintf(stderr, "Error: %s at line %d\n", s,line);
    exit(EXIT_FAILURE);
}

void getMips (FILE *f, symbol *s, quad *q) {
    // data
    pdir(".data");
    pdat("_true", ".asciiz \"true\"");
    pdat("_false", ".asciiz \"false\"");
    pdat("_read_int", ".asciiz \"Enter int: \"");
    pdat("_segfault_mess", ".asciiz \"Invalid read of size 4\n\"");

    // tos global
    pdir("");
    pcom("TOS Global");
    getData(f, s);

    // tos of each function
    while (s != NULL) {
        if (s->type == S_FUNCTION) {
            fundata *fdata = (fundata *) s->fdata;

            snpt(snprintf(tbuf, LEN, "TOS of function %s", s->id));
            pcom(tbuf);
            getData(f, fdata->tos);
        }

        s = s->next;
    }

    // text
    pdir("\n.text");
    pdir(".globl main");

    pdir("");
    pcom("Internal functions")
    getMipsInternalFunctions(f);

    pdir("");
    pcom("User functions");
    getText(f, q);

    pcom("Exit program");
    pins2("j", "_exit");
}

void getMipsInternalFunctions (FILE *f) {
    // quitter le programme proprement
    plab("_exit");
    pins3("li", "$v0", "10");
    pins1("syscall");

    // if segfault go HERE
    plab("_segfault");
    pins3("li", "$v0", "4");
    pins3("la", "$a0", "_segfault_mess");
    pins1("syscall");
    pins2("j", "_exit");

    // helper to compute array index
    getMipsFunctionCompind(f);
}

void getMipsFunctionCompind (FILE *f) {
    /**
     * Function compind = compute array index with given args and props
     * Call this function after all these registers are set correctly:
     * $t8: index init to 0, $t7: factor init to 1
     * $t0: cur ind, $t1: dim min, $t2: dim max  -> they should change on each call
     * Call this function for each dim/args in reverse order
     * ex: tab[1,2] => call for ind "2" and next call for ind "1"
     */

    // check if segfault
    plab("_compind");
    pins4("blt", "$t0", "$t1", "_segfault");
    pins4("bgt", "$t0", "$t2", "_segfault");

    // Calcul de index
    pins4("sub", "$t3", "$t0", "$t1");
    pins4("mul", "$t4", "$t3", "$t7");
    pins4("add", "$t8", "$t8", "$t4");

    // Calcul de factor
    pins4("sub", "$t3", "$t2", "$t1");
    pins4("add", "$t4", "$t3", "1");
    pins4("mul", "$t7", "$t7", "$t4");
    pins2("jr", "$ra");
}

void getData (FILE *f, symbol *s) {
    int i;
    while (s != NULL) {
        if (s->ref) {
            pdat(s->id, ".word 0");
        } else {
            switch (s->type) {
                case S_INT:
                    snpt(snprintf(tbuf, LEN, ".word %d", s->ival));
                    pdat(s->id, tbuf);
                    break;
                case S_BOOL:
                    snpt(snprintf(tbuf, LEN, ".byte %d", s->bval));
                    pdat(s->id, tbuf);
                    break;
                case S_STRING:
                    snpt(snprintf(tbuf, LEN, ".asciiz %s", s->sval));
                    pdat(s->id, tbuf);
                    break;
                case S_ARRAY:
                    fprintf(f, "\t%s:\t.%s ", s->id, s->arr->type == S_INT ? "word" : "byte");

                    for (i = 0; i < s->arr->size; i++) {
                        if (i != s->arr->size - 1)
                        fprintf(f, "0, ");
                        else
                        fprintf(f, "0\n");
                    }

                    break;
                default:
                    //TO AVOID WARNINGS
                    break;
            }
        }

        s = s->next;
    }
}

char * opstr (qop op) {
    static char str[LEN]; // NOT THREAD SAFE
    switch (op) {
            case Q_PLUS  : sprintf(str, "+")   ; break ;
            case Q_MINUS : sprintf(str, "-")   ; break ;
            case Q_MULT  : sprintf(str, "*")   ; break ;
            case Q_DIV   : sprintf(str, "/")   ; break ;
            case Q_MOD   : sprintf(str, "MOD") ; break ;
            case Q_EXP   : sprintf(str, "^")   ; break ;
            case Q_EQUAL : sprintf(str, "=")   ; break ;
            case Q_DIFF  : sprintf(str, "!=")  ; break ;
            case Q_INF   : sprintf(str, "<")   ; break ;
            case Q_INFEQ : sprintf(str, "<=")  ; break ;
            case Q_SUP   : sprintf(str, ">")   ; break ;
            case Q_SUPEQ : sprintf(str, ">=")  ; break ;
            case Q_AND   : sprintf(str, "AND") ; break ;
            case Q_OR    : sprintf(str, "OR")  ; break ;
            case Q_XOR   : sprintf(str, "XOR") ; break ;
            case Q_NOT   : sprintf(str, "NOT") ; break ;
            default: ferr(__LINE__ ,"mips.c opstr unknow op");
    }

    return str;
}

char * nextTmpLabel (void) {
    static int nlabel = 0;
    char tmplab[LEN];
    sprintf(tmplab, "tmplab_%d", nlabel ++);

    char *res = strdup(tmplab);
    if (res == NULL)
        ferr(__LINE__ ,"mips.c nextTmpLabel strdup");

    return res;
}

void getText (FILE *f, quad *q) {
    symbol *res, *argv1, *argv2, *gtrue, *gfalse, *gnext;

    while (q != NULL) {
        curquad = q;
        res     = q->res;
        argv1   = q->argv1;
        argv2   = q->argv2;
        gtrue   = q->gtrue;
        gfalse  = q->gfalse;
        gnext   = q->gnext;
        (void)gtrue;
        (void)gnext;
        switch (q->op) {
            case Q_PLUS:
            case Q_MINUS:
            case Q_MULT:
            case Q_DIV:
            case Q_MOD:
            case Q_EXP:
                if (!res || !argv1 || !argv2)
                    ferr(__LINE__ ,"mips.c getText arith quad error");

                qArith(f, q->op, res, argv1, argv2);
                break;

            case Q_WRITE:
                if (!argv1)
                    ferr(__LINE__ ,"mips.c getText Q_WRITE quad error");

                qWrite(f, argv1);
                break;

            case Q_READ:
                if (!res)
                    ferr(__LINE__ ,"mips.c getText Q_READ quad error");

                // argv1 only for array = for index calculation
                qRead(f, res, argv1);
                break;

            case Q_AFFEC:
                if (!res || !argv1)
                    ferr(__LINE__ ,"mips.c getText Q_AFFEC quad error");

                // argv2 only for array = for index calculation
                qAffect(f, res, argv1, argv2);
                break;

            case Q_LABEL:
                if (!res)
                    ferr(__LINE__ ,"mips.c getText Q_LABEL quad error");

                plab(res->id);
                break;

            case Q_GOTO:
                if (!res)
                    ferr(__LINE__ ,"mips.c getText Q_GOTO quad error");

                snpt(snprintf(tbuf, LEN, "goto %s", res->id));
                pcom(tbuf);

                pins2("j", res->id);
                break;

            case Q_IF:
                if (!argv1 || !gfalse)
                    ferr(__LINE__ ,"mips.c getText Q_IF quad error");

                snpt(snprintf(tbuf, LEN, "if %s is false then goto %s", dsymid(argv1), gfalse->sval));
                pcom(tbuf);

                pload("$t0", argv1);
                pins4("beq", "$t0", "$zero", gfalse->sval);
                break;

            case Q_EQUAL:
            case Q_DIFF:
            case Q_INF:
            case Q_INFEQ:
            case Q_SUP:
            case Q_SUPEQ:
            case Q_AND:
            case Q_OR:
            case Q_XOR:
                if (!res || !argv1 || !argv2)
                    ferr(__LINE__ ,"mips.c getText comp quad error");

                qComp(f, q->op, res, argv1, argv2);
                break;

            case Q_NOT:
                if (!res || !argv1)
                    ferr(__LINE__ ,"mips.c getText Q_NOT quad error");

                qNot(f, res, argv1);
                break;

            case Q_FUNDEC:
                // argv1 = function symbol
                if (!argv1)
                    ferr(__LINE__ ,"mips.c getText Q_FUNDEC quad error");

                fundec(f, argv1);
                break;

            case Q_FUNEND:
                // argv1 = function symbol
                if (!argv1)
                    ferr(__LINE__ ,"mips.c getText Q_FUNEND quad error");

                funend(f, argv1);
                break;

            case Q_FUNCALL:
                // res   = where to put function return value (can be null = fun type S_UNIT)
                // arvg1 = function symbol
                // argv2 = list of symbol = symbol * (can be null = no args)
                if (!argv1)
                    ferr(__LINE__ ,"mips.c getText Q_FUNCALL quad error");

                funcall(f, argv1, argv2, res);
                break;

            case Q_FUNRETURN:
                // arvg1 = function symbol
                // arvg2 = symbol to return (can be NULL if fun unit)
                if (!argv1)
                    ferr(__LINE__ ,"mips.c getText Q_FUNRETURN quad error");

                funreturn(f, argv1, argv2);
                break;

            case Q_MAIN:
                plab("main");
                break;

            default:
                ferr(__LINE__ ,"mips.c getText unknown op");
        }

        q = q->next;
    }
}

////////////
// COMMON //
////////////

void qAffect (FILE *f, symbol *res, symbol *argv1, symbol *argv2) {
    // argv2 only for array = for index calculation

    if (res->type == S_ARRAY) {
        arrComputeIndex(f, res, argv2);
        // index in $t8
        snpt(snprintf(tbuf, LEN, "%s[%s] := %s", res->id, "$t8", argv1->id));
        pcom(tbuf);

        if (res->ref) {
            pins3("lw", "$t3", res->id);
        } else {
            pins3("la", "$t3", res->id);
        }

        if (res->arr->type == S_INT) {
            pins4("mul", "$t4", "$t8", "4");
            pins4("add", "$t1", "$t4", "$t3" );
        } else
            pins4("add", "$t1", "$t8", "$t3");

        pload("$t5", argv1);
        pins3(argv1->type == S_BOOL ? "sb" : "sw", "$t5", "($t1)");

    } else if (argv1->type == S_ARRAY) {
        arrComputeIndex(f, argv1, argv2);
        // index in $t8
        snpt(snprintf(tbuf, LEN, "%s := %s[%s]", res->id, argv1->id, "$t8"));
        pcom(tbuf);

        if (argv1->ref) {
            pins3("lw", "$t3", argv1->id);
        } else {
            pins3("la", "$t3", argv1->id);
        }

        if (argv1->arr->type == S_INT) {
            pins4("mul", "$t4", "$t8", "4");
            pins4("add", "$t1", "$t4", "$t3");
        } else
            pins4("add", "$t1", "$t8", "$t3");

        pins3(res->type == S_BOOL ? "lb" : "lw", "$t5", "($t1)");
        pstore("$t5", res);

    } else {
        snpt(snprintf(tbuf, LEN, "%s := %s", dsymid(res), dsymid(argv1)));
        pcom(tbuf);

        pload("$t0", argv1);
        pstore("$t0", res);
    }
}

void qRead (FILE *f, symbol *res, symbol *argv1) {
    // argv1 only for array = for index calculation
    char *label, *label2;

    snpt(snprintf(tbuf, LEN, "read int %s", dsymid(res)));
    pcom(tbuf);

    pins3("li", "$v0", "4");
    pins3("la", "$a0", "_read_int");
    pins1("syscall");
    pins3("li", "$v0", "5");
    pins1("syscall");

    switch (res->type) {
        case S_INT:
            pstore("$v0", res);
            break;

        case S_BOOL:
            label  = nextTmpLabel();
            label2 = nextTmpLabel();

            pins4("beq", "$v0", "$zero", label);
            pins3("li", "$t0", "1");
            pins2("j", label2);

            plab(label);
            pins3("li", "$t0", "0");

            plab(label2);
            pstore("$t0", res);

            free(label);
            free(label2);
            break;

        case S_ARRAY:
            arrComputeIndex(f, res, argv1);
            // index in $t8

            pins3("la", "$t3", res->id);

            if (res->arr->type == S_INT) {
                pins4("mul", "$t4", "$t8", "4");
                pins4("add", "$t1", "$t4", "$t3");
            } else
                pins4("add", "$t1", "$t8", "$t3");

            pins3("sw", "$v0", "($t1)");
            break;

        default: // TO AVOID WARNIGS
            break;
    }
}

void qWrite (FILE *f, symbol *argv1) {
    char *label, *label2;

    switch (argv1->type) {
        case S_INT:
            snpt(snprintf(tbuf, LEN, "print integer %s", dsymid(argv1)));
            pcom(tbuf);

            pins3("li", "$v0", "1");
            pload("$a0", argv1);
            break;

        case S_STRING:
            snpt(snprintf(tbuf, LEN, "print string %s", argv1->id));
            pcom(tbuf);

            pins3("li", "$v0", "4");
            pins3("la", "$a0", argv1->id);
            if(!argv1->tmp)
                pins3("move", "$a0", "$s0");
            break;

        case S_BOOL:
            label  = nextTmpLabel();
            label2 = nextTmpLabel();

            snpt(snprintf(tbuf, LEN, "print bool %s", dsymid(argv1)));
            pcom(tbuf);

            pload("$t0", argv1);
            pins4("beq", "$t0", "$zero", label);
            pins3("la", "$a0", "_true");
            pins2("j", label2);

            plab(label);
            pins3("la", "$a0", "_false");

            plab(label2);
            pins3("li", "$v0", "4");
            break;
        default:
            //TO AVOID WARNINGS
            break;
    }

    pins1("syscall");
}

void qArith (FILE *f, qop op, symbol *res, symbol *argv1, symbol *argv2) {
    snpt(snprintf(tbuf, LEN, "%s := %s %s %s", dsymid(res), dsymid(argv1), opstr(op), dsymid(argv2)));
    pcom(tbuf);

    pload("$t0", argv1);
    pload("$t1", argv2);

    char *label, *label2;
    switch (op) {
        case Q_PLUS  : pins4("add", "$t2", "$t0", "$t1") ; break ;
        case Q_MINUS : pins4("sub", "$t2", "$t0", "$t1") ; break ;
        case Q_MULT  : pins4("mul", "$t2", "$t0", "$t1") ; break ;
        case Q_DIV   : pins4("div", "$t2", "$t0", "$t1") ; break ;
        case Q_MOD   : pins4("rem", "$t2", "$t0", "$t1") ; break ;
        case Q_EXP   :
           label  = nextTmpLabel();
           label2 = nextTmpLabel();

           pload("$t2", argv1);
           pins3("li", "$t3", "1");

           // warning: seulement les puissances > 0 fonctionnent avec ce code
           plab(label);
           pins4("ble", "$t1", "$t3", label2);
           pins4("mul", "$t2", "$t2", "$t0");
           pins4("sub", "$t1", "$t1", "$t3");
           pins2("j", label);
           plab(label2);

           free(label);
           free(label2);
           break;

        default:
            //TO AVOID WARNING
            break;
    }

    pstore("$t2", res);
}

void qComp (FILE *f, qop op, symbol *res, symbol *argv1, symbol *argv2) {
    char *label, *label2;
    label  = nextTmpLabel();
    label2 = nextTmpLabel();

    snpt(snprintf(tbuf, LEN, "%s := %s %s %s", dsymid(res), dsymid(argv1), opstr(op), dsymid(argv2)));
    pcom(tbuf);

    pload("$t0", argv1);
    pload("$t1", argv2);

    switch (op) {
        case Q_EQUAL:
            pins4("bne", "$t0", "$t1", label);
            break;
        case Q_DIFF:
            pins4("beq", "$t0", "$t1", label); break;
        case Q_INF:
            pins4("bge", "$t0", "$t1", label);
            break;
        case Q_INFEQ:
            pins4("bgt", "$t0", "$t1", label);
            break;
        case Q_SUP:
            pins4("ble", "$t0", "$t1", label);
            break;
        case Q_SUPEQ:
            pins4("blt", "$t0", "$t1", label);
            pins4("beq", "$t2", "$zero", label);
            break;
        case Q_OR:
            pins4("or", "$t2", "$t0", "$t1");
            pins4("beq", "$t2", "$zero", label);
            break;
        case Q_AND:
            pins4("and", "$t2", "$t0", "$t1");
            pins4("beq", "$t2", "$zero", label);
            break;
        case Q_XOR:
            pins4("xor", "$t2", "$t0", "$t1");
            pins4("beq", "$t2", "$zero", label);
            break;
        default:
            ferr(__LINE__ ,"mips.c qComp wrong op");
    }

    pins3("li", "$t3", "1");
    pins2("j", label2);

    plab(label);
    pins3("li", "$t3", "0");

    plab(label2);
    pstore("$t3", res);

    free(label);
    free(label2);
}

void qNot (FILE *f, symbol *res, symbol *argv1) {
    char *label, *label2;
    label  = nextTmpLabel();
    label2 = nextTmpLabel();

    snpt(snprintf(tbuf, LEN, "%s := NOT %s", dsymid(res), dsymid(argv1)));
    pcom(tbuf);

    pload("$t0", argv1);
    pins4("bne", "$t0", "$zero", label);
    pins3("li", "$t3", "1");
    pins2("j", label2);

    plab(label);
    pins3("li", "$t3", "0");

    plab(label2);
    pstore("$t3", res);

    free(label);
    free(label2);
}

/**
 * Compute array index from sargs->args and sarr->arr->dims
 * The index is put in register $t8
 */
void arrComputeIndex (FILE *f, symbol *sarr, symbol *sargs) {
    snpt(snprintf(tbuf, LEN, "Compute array index of %s", sarr->id));
    pcom(tbuf);

    arglist *lal = sargs->args;
    dimProp *ldp = sarr->arr->dims;

    rlist *rlal = rlistNew(lal, NULL);
    rlist *rldp = rlistNew(NULL, ldp);

    pins3("li", "$t8", "0"); // $t8 = index
    pins3("li", "$t7", "1"); // $t7 = factor

    while (rlal && rldp) {
        pins3("lw", "$t0", rlal->al->sym->id); // $t0 cur ind
        snpt(snprintf(tbuf, LEN, "%d", rldp->dp->min));
        pins3("li", "$t1", tbuf); // $t1 min
        snpt(snprintf(tbuf, LEN, "%d", rldp->dp->max));
        pins3("li", "$t2", tbuf); // $t2 max
        pins2("jal", "_compind");

        rlal = rlal->next;
        rldp = rldp->next;
    }

    snpt(snprintf(tbuf, LEN, "%d", sarr->arr->size));
    pins3("li", "$t9", tbuf);
    pins4("bgt", "$t8", "$t9", "_segfault");
}


///////////////
// FUNCTIONS //
///////////////

void fundec (FILE *f, symbol *fun) {
    plab(fun->id);

    // sauvegardage du ra
    pcom("push $ra to stack and load each arg from stack");

    pins4("sub", "$sp", "$sp", "4");
    pins3("sw", "$ra", "0($sp)");

    // load args from stack offset 4
    funStackLoadArgs(f, fun, 4);

    snpt(snprintf(tbuf, LEN, "body of function %s", fun->id));
    pcom(tbuf);

    /* Stack now:
       0 -> ra
       4 -> arg 1
       8 -> arg 2
       etc ...
    */

    curfun = fun;
}

void funend (FILE *f, symbol *fun) {
    /* Stack now:
       0 -> ra
       4 -> arg 1
       8 -> arg 2
       etc ...
    */

    // label to jump to after a return
    snpt(snprintf(tbuf, LEN, "end_%s", fun->id));
    plab(tbuf);
    snpt(snprintf(tbuf, LEN, "epilogue of function %s", fun->id));
    pcom(tbuf);

    // load saved ra to $ra
    pins3("lw", "$ra", "0($sp)");

    int offset = 4 + funArgsSize(fun);
    // pop ra and args from the stack
    snpt(snprintf(tbuf, LEN, "%d", offset));
    pins4("addi", "$sp", "$sp", tbuf);

    // jump to $ra
    pins2("jr", "$ra");
    snpt(snprintf(tbuf, LEN, "end of function %s", fun->id));
    pcom(tbuf);

    curfun = NULL;
}

void funcall (FILE *f, symbol *fun, symbol *args, symbol *res) {
    // caller push return adress + args to the stack if any, callee pop them before returning to $ra
    int size;

    if (curfun != NULL) {
        pcom("push usefull local vars to the stack for funcall");
        // make space for local var save in the stack
        size = curfunVarSize();
        snpt(snprintf(tbuf, LEN, "%d", size));
        pins4("sub", "$sp", "$sp", tbuf);

        // push local vars to the stack
        curfunStackPushVars(f);
    }

    pcom("push funcall args to the stack");
    // make space for args in the stack
    size = funArgsSize(fun);
    snpt(snprintf(tbuf, LEN, "%d", size));
    pins4("sub", "$sp", "$sp", tbuf);

    // push args to stack
    funStackPushArgs(f, fun, args);

    /* Stack now (no curfun)    Stack now (curfun != NULL)
       0 -> arg 1               0 -> arg 1
       4 -> arg 2               4 -> arg 2
         etc ...                  etc ...
                                8  -> var1
                                12 -> var2
                                etc ...
    */

    // show funcall args string for debugging
    char argsDebug[LEN];
    funArgsDebugString(fun, args, argsDebug, LEN);

    if (res) {
        snpt(snprintf(tbuf, LEN, "funcall %s := %s ( %s )", dsymid(res), fun->id, argsDebug));
    } else {
        snpt(snprintf(tbuf, LEN, "funcall %s ( %s )", fun->id, argsDebug));
    }

    pcom(tbuf);

    // jump to function and put actual addr in $ra
    pins2("jal", fun->id);

    if (curfun != NULL) {
        /* Stack now (curfun != NULL)
           0 -> var1
           4 -> var2
           etc ...
        */

        pcom("load usefull local vars from the stack");
        // load local vars from stack
        curfunStackLoadVars(f);
        size = curfunVarSize();

        // pop local vars from stack
        snpt(snprintf(tbuf, LEN, "%d", size));
        pins4("addi", "$sp", "$sp", tbuf);
    }

    // store result ($v0) in res->id
    if (res)
        pins3("sw", "$v0", res->id);

}

void funreturn (FILE *f, symbol *fun, symbol *ret) {
    if (ret) {
        snpt(snprintf(tbuf, LEN, "funreturn %s", ret->id));
        pcom(tbuf);
        pins3("lw", "$v0", ret->id);
    } else
        pcom("funreturn (void)");

    // goto function end label
    snpt(snprintf(tbuf, LEN, "end_%s", fun->id));
    pins2("j", tbuf);
}

//////////////////////
// FUNCTION HELPERS //
//////////////////////

int funArgsSize (symbol *fun) {
    int size = 0, bytes;
    arglist *al = ((fundata *) fun->fdata)->al;

    while (al != NULL) {
        bytes = funSymTypeSize(al->sym);
        size += bytes;
        al    = al->next;
    }

    return size;
}

int funSymTypeSize (symbol *sym) {
    int bytes;

    if (sym->ref)
        bytes = 4;
    else {
        switch (sym->type) {
            case S_INT    : bytes = 4 ; break ;
            case S_BOOL   : bytes = 1 ; break ;
            case S_ARRAY  : bytes = 4;   break ;
            default       : ferr(__LINE__, "mips.c funSymTypeSize arg wrong type");

        }
    }

    return bytes;
}

/**
 * @param offset Starting offset (= after saved ra)
 */
void funStackLoadArgs (FILE *f, symbol *fun, int offset) {
    int bytes;
    arglist *al = ((fundata *) fun->fdata)->al;

    while (al != NULL) {
        snpt(snprintf(tbuf, LEN, "%d($sp)", offset));
        pins3("lw", "$t0", tbuf);

        if (al->sym->type == S_ARRAY && !al->sym->ref) {
            // copy original array to function local array
            funCopyArray(f, al->sym);
        } else {
            pins3("sw", "$t0", al->sym->id);
        }

        bytes   = funSymTypeSize(al->sym);
        offset += bytes;
        al      = al->next;
    }
}

/**
 * @info $t0 should be the register which contains the address of the original array to copy from
 * @param destSym Symbol array to copy data to (function parameter local array symbol)
 */
void funCopyArray (FILE *f, symbol *destSym) {
    int i, off = 0;
    int addOff = destSym->arr->type == S_INT ? 4 : 1;

    snpt(snprintf(tbuf, LEN, "copy array ($t0) to %s", destSym->id));
    pcom(tbuf);
    pins3("la", "$t1", destSym->id);

    char lins[3], sins[3];
    if (destSym->arr->type == S_INT) {
        sprintf(lins, "lw");
        sprintf(sins, "sw");
    } else {
        sprintf(lins, "lb");
        sprintf(sins, "sb");
    }

    for (i = 0; i < destSym->arr->size; i ++) {
        snpt(snprintf(tbuf, LEN, "%d($t0)", off));
        pins3(lins, "$t2", tbuf);

        snpt(snprintf(tbuf, LEN, "%d($t1)", off));
        pins3(sins, "$t2", tbuf);

        off += addOff;
    }
}

void funStackPushArgs (FILE *f, symbol *fun, symbol *args) {
    int offset  = 0, bytes;
    arglist *al = ((fundata *) fun->fdata)->al;

    while (args != NULL) {
        if (al == NULL)
            ferr(__LINE__ , "mips.c funStackPushArgs args len > fun param len");

        if (args->type != al->sym->type)
            ferr(__LINE__ , "mips.c funStackPushArgs arg type != fun param type");

        if (args->type == S_ARRAY) {
            if (args->arr->type != al->sym->arr->type)
                ferr(__LINE__, "mips.c funStackPushArgs type array but array elems type differ");
            if (args->arr->size != al->sym->arr->size)
                ferr(__LINE__, "mips.c funStackPushArgs type array but array size differ");
        }

        if (al->sym->ref || al->sym->type == S_ARRAY) {
            pins3("la", "$t0", args->id);
        } else {
            pins3("lw", "$t0", args->id);
        }

        snpt(snprintf(tbuf, LEN, "%d($sp)", offset));
        pins3("sw", "$t0", tbuf);

        bytes   = funSymTypeSize(args);
        offset += bytes;
        args    = args->next;
        al      = al->next;
    }

    if (al != NULL)
        ferr(__LINE__ ,"mips.c funStackPushArgs args len < fun param len");
}

void funArgsDebugString (symbol *fun, symbol *args, char *dstring, int maxlen) {
    int bytes, len = 0;
    arglist *al = ((fundata *) fun->fdata)->al;

    while (args != NULL) {
        if (al == NULL)
            ferr(__LINE__ ,"mips.c funArgsDebugString args len > fun param len");

        bytes = snprintf(dstring + len, maxlen - len, "%s%s, ", al->sym->ref ? "&" : "", args->id);
        if (bytes < 0 || bytes >= maxlen - len)
            ferr(__LINE__ ,"mips.c funArgsDebugString snprintf");

        len += bytes;
        args = args->next;
        al   = al->next;
    }

    if (al != NULL)
        ferr(__LINE__ ,"mips.c funArgsDebugString args len < fun param len");

    if (len > 2)
        dstring[len - 2] = '\0'; // erase the last ", "
}

// curfun functions (only for function calls inside a function)

int curfunVarSize (void) {
    if (curfun == NULL)
        ferr(__LINE__ ,"mips.c curfunVarSize - curfun is NULL");

    int size = 0, bytes;
    symbol *tos = ((fundata *) curfun->fdata)->tos;

    while ((tos = curfunNextUsefullLocalVar(tos)) != NULL) {
        bytes = funSymTypeSize(tos);
        size += bytes;
        tos   = tos->next;
    }

    return size;
}

void curfunStackPushVars (FILE *f) {
    if (curfun == NULL)
        ferr(__LINE__ ,"mips.c curfunStackPushVars - curfun is NULL");

    int offset = 0, bytes;
    symbol *tos = ((fundata *) curfun->fdata)->tos;

    while ((tos = curfunNextUsefullLocalVar(tos)) != NULL) {
        pins3("lw", "$t0", tos->id);
        snpt(snprintf(tbuf, LEN, "%d($sp)", offset));
        pins3("sw", "$t0", tbuf);

        bytes   = funSymTypeSize(tos);
        offset += bytes;
        tos     = tos->next;
    }
}

void curfunStackLoadVars (FILE *f) {
    if (curfun == NULL)
        ferr(__LINE__ ,"mips.c curfunStackLoadVars - curfun is NULL");

    int offset = 0, bytes;
    symbol *tos = ((fundata *) curfun->fdata)->tos;

    while ((tos = curfunNextUsefullLocalVar(tos)) != NULL) {
        snpt(snprintf(tbuf, LEN, "%d($sp)", offset));
        pins3("lw", "$t0", tbuf);
        pins3("sw", "$t0", tos->id);

        bytes   = funSymTypeSize(tos);
        offset += bytes;
        tos     = tos->next;
    }
}

/**
 * This function is used to get the next local var of a function tos, which is usefull
 * to save in the stack before the funcall and reload it again after
 * The returned symbol should be push to the stack and call again this function with the result->next until the function returns NULL
 *
 * Returned symbols are used after the funcall in argument of an operation
 *
 * @param tos Current symbol in the curfun tos
 */
symbol * curfunNextUsefullLocalVar (symbol *tos) {
    if (curfun == NULL)
        ferr(__LINE__ ,"mips.c curfunNextUsefullLocalVar - curfun is NULL");

    quad *q;

    while (tos != NULL) {
        if (tos->type != S_INT && tos->type != S_BOOL) {
            tos = tos->next;
            continue;
        }

        q = curquad;
        // check if this symbol is used in an operation after the funcall

        while (q->op != Q_FUNEND) {
            if (q->argv1 == tos || q->argv2 == tos)
                return tos;

            q = q->next;
        }

        tos = tos->next;
    }

    return NULL;
}

/**
 * -----------------------------------
 * ###### AIDE GESTION DU STACK ######
 * -----------------------------------
 * - l'appelant empile les arguments de l'appel de fonction (1er argument en sommet de pile) et jump sur la fonction
 * - l'appelé empile par dessus l'adresse de retour depuis $ra
 * - l'appelé charge les arguments de l'appel de fonction dans les symboles de sa table des symboles
 * - A la fin de la fonction, l'appelé place le résultat (si fonction type != unit) dans $v0, que l'appelant récupère ensuite)
 * - l'appelé charge ensuite l'adresse de retour sauvegardée (en sommet de pile) du stack vers son registre $ra, puis la dépile du stack ainsi que les arguments initiaux de l'appel de fonction
 *   l'appelé jump sur $ra, pour revenir juste après l'appel de fonction initial
 *
 *  -----------------------
 *  Si l'appel de fonction se déroule à l'intérieur d'une fonction (= ailleur que dans le main), cela se rajoute:
 *  - l'appelant sauvegarde (empile) toutes les variables INT ou BOOL utiles (= utilisées après l'appel de fonction) de sa table des symboles (tos) dans le stack (juste en-dessous des arguments de l'appel de fonction, avec la 1ere var locale utile juste après le dernier argument)
 *  - l'appelé ne touchera pas à ces variables locales sauvegardées
 *  - Une fois de retour juste après l'appel de fonction, l'appelant restaure ses variables locales utiles depuis le stack et les dépiles de celui-ci.
 */
