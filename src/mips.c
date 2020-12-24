#include "../headers/mips.h"

// print instr 1 arg
#define pinstr1(s1) \
    fprintf(f, "\t%s\n", s1);

// print instr 2 args
#define pinstr2(s1, s2) \
    fprintf(f, "\t%-5s %s\n", s1, s2);

// print instr 3 args
#define pinstr3(s1, s2, s3) \
    fprintf(f, "\t%-5s %-5s %s\n", s1, s2, s3);

// print instr 4 args
#define pinstr4(s1, s2, s3, s4) \
    fprintf(f, "\t%-5s %-5s %-5s %s\n", s1, s2, s3, s4);

// print data
#define pdata(s1, s2) \
    fprintf(f, "\t%-16s :  %s\n", s1, s2);

// print label
#define plabel(s) \
    fprintf(f, "\n%s:\n", s);

// print commentary
#define pcom(s) \
    fprintf(f, "\t\t\t# %s\n", s);

// print directive
#define pdir(s) \
    fprintf(f, "%s\n", s);

static char tbuf[LEN];
static symbol *curfun = NULL;

/**
 * curfun is used to know if a funcall is done inside a function or not
 * If a funcall is done inside a function, save all function local vars to the stack and restore them after the call
 */

void ferr (char *s) {
    fprintf(stderr, "Error: %s\n", s);
    exit(EXIT_FAILURE);
}

void getMips (FILE *f, symbol *s, quad *q) {
    // data
    pdir(".data");
    pdata("_true", ".asciiz \"true\"");
    pdata("_false", ".asciiz \"false\"");
    pdata("_read_int", ".asciiz \"Enter int: \"");
    pdata("_read_string", ".asciiz \"Enter string: \"");
    pdir(".align 2");
    snprintf(tbuf, LEN, ".space %d", MIPS_BUFFER_SPACE);
    pdata("_buffer", tbuf);

    // tos global
    pdir("");
    pcom("TOS Global");
    getData(f, s);

    // tos of each function
    while (s != NULL) {
        if (s->type == S_FUNCTION) {
            fundata *fdata = (fundata *) s->fdata;

            snprintf(tbuf, LEN, "TOS of function %s", s->id);
            pcom(tbuf);
            getData(f, fdata->tos);
        }

        s = s->next;
    }

    // text
    pdir("\n.text");
    pdir(".globl main");
    getText(f, q);

    // quitter le programme proprement
    plabel("exit");
    pinstr3("li", "$v0", "10");
    pinstr1("syscall");
}

void getData (FILE *f, symbol *s) {
    while (s != NULL) {
        switch (s->type) {
            case S_INT:
                snprintf(tbuf, LEN, ".word %d", s->ival);
                pdata(s->id, tbuf);
                break;
            case S_BOOL:
                snprintf(tbuf, LEN, ".word %d", s->bval);
                pdata(s->id, tbuf);
                break;
            case S_STRING:
                snprintf(tbuf, LEN, ".asciiz %s", s->sval);
                pdata(s->id, tbuf);
                break;
            case S_ARRAY:
                if (s->arr->type == S_INT) {
                    snprintf(tbuf, LEN, ".space %d", s->arr->size * 4);
                    pdata(s->id, tbuf);
                }
                break;
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
            default: ferr("mips.c opstr unknow op");
    }

    return str;
}

char * nextTmpLabel () {
    static int nlabel = 0;
    char tmplabel[LEN];
    sprintf(tmplabel, "tmplabel_%d", nlabel ++);

    char *res = strdup(tmplabel);
    if (res == NULL)
        ferr("mips.c nextTmpLabel strdup");

    return res;
}

void getText (FILE *f, quad *q) {
    symbol *res, *argv1, *argv2, *gtrue, *gfalse, *gnext;

    while (q != NULL) {
        res    = q->res;
        argv1  = q->argv1;
        argv2  = q->argv2;
        gtrue  = q->gtrue;
        gfalse = q->gfalse;
        gnext  = q->gnext;

        switch (q->op) {
            case Q_PLUS:
            case Q_MINUS:
            case Q_MULT:
            case Q_DIV:
            case Q_MOD:
            case Q_EXP:
                if (!res || !argv1 || !argv2)
                    ferr("mips.c getText arith quad error");

                qArith(f, q->op, res, argv1, argv2);
                break;

            case Q_WRITE:
                if (!argv1)
                    ferr("mips.c getText Q_WRITE quad error");

                qWrite(f, argv1);
                break;

            case Q_READ:
                if (!res)
                    ferr("mips.c getText Q_READ quad error");

                qRead(f, res);
                break;

            case Q_AFFEC:
                if (!res || !argv1)
                    ferr("mips.c getText Q_AFFEC quad error");

                snprintf(tbuf, LEN, "%s := %s", res->id, argv1->id);
                pcom(tbuf);

                pinstr3("lw", "$t0", argv1->id);
                pinstr3("sw", "$t0", res->id);
                break;

            case Q_LABEL:
                if (!res)
                    ferr("mips.c getText Q_LABEL quad error");

                plabel(res->id);
                break;

            case Q_GOTO:
                if (!res)
                    ferr("mips.c getText Q_GOTO quad error");

                snprintf(tbuf, LEN, "goto %s", res->id);
                pcom(tbuf);

                pinstr2("j", res->id);
                break;

            case Q_IF:
                if (!argv1 || !gtrue || !gfalse || !gnext)
                    ferr("mips.c getText Q_IF quad error");

                snprintf(tbuf, LEN, "if %s is false then goto %s", argv1->id, gfalse->sval);
                pcom(tbuf);

                pinstr3("lw", "$t0", argv1->id);
                pinstr4("beq", "$t0", "$zero", gfalse->sval);
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
                    ferr("mips.c getText comp quad error");

                qComp(f, q->op, res, argv1, argv2);
                break;

            case Q_NOT:
                if (!res || !argv1)
                    ferr("mips.c getText Q_NOT quad error");

                qNot(f, res, argv1);
                break;

            case Q_FUNDEC:
                // argv1 = function symbol
                if (!argv1)
                    ferr("mips.c getText Q_FUNDEC quad error");

                fundec(f, argv1);
                break;

            case Q_FUNEND:
                // argv1 = function symbol
                if (!argv1)
                    ferr("mips.c getText Q_FUNEND quad error");

                funend(f, argv1);
                break;

            case Q_FUNCALL:
                // res   = where to put function return value (can be null = fun type S_UNIT)
                // arvg1 = function symbol
                // argv2 = list of symbol = symbol * (can be null = no args)
                if (!argv1)
                    ferr("mips.c getText Q_FUNCALL quad error");

                funcall(f, argv1, argv2, res);
                break;

            case Q_FUNRETURN:
                // arvg1 = function symbol
                // arvg2 = symbol to return (can be NULL if fun unit)
                if (!argv1)
                    ferr("mips.c getText Q_FUNRETURN quad error");

                funreturn(f, argv1, argv2);
                break;

            case Q_MAIN:
                pcom("main function");
                plabel("main");
                break;

            default:
                ferr("mips.c getText unknown op");
        }

        q = q->next;
    }
}

////////////
// COMMON //
////////////

void qRead (FILE *f, symbol *res) {
    char *label, *label2;

    switch (res->type) {
        case S_INT:
            snprintf(tbuf, LEN, "read integer %s", res->id);
            pcom(tbuf);

            pinstr3("li", "$v0", "4");
            pinstr3("la", "$a0", "_read_int");
            pinstr1("syscall");
            pinstr3("li", "$v0", "5");
            pinstr1("syscall");
            pinstr3("sw", "$v0", res->id);
            break;

        case S_STRING:
            snprintf(tbuf, LEN, "read string %s", res->id);
            pcom(tbuf);

            pinstr3("li", "$v0", "4");
            pinstr3("la", "$a0", "_read_string");
            pinstr1("syscall");
            pinstr3("li", "$v0", "8");
            pinstr3("la", "$a0", "_buffer");
            snprintf(tbuf, LEN, "%d", MIPS_BUFFER_SPACE);
            pinstr3("li", "$a1", tbuf);
            pinstr3("move", "$s0", "$a0");
            pinstr1("syscall");
            break;

        case S_BOOL:
            label  = nextTmpLabel();
            label2 = nextTmpLabel();

            snprintf(tbuf, LEN, "read bool %s", res->id);
            pcom(tbuf);

            pinstr3("li", "$v0", "4");
            pinstr3("la", "$a0", "_read_int");
            pinstr1("syscall");
            pinstr3("li", "$v0", "5");
            pinstr1("syscall");

            pinstr4("beq", "$v0", "$zero", label);
            pinstr3("li", "$t0", "1");
            pinstr2("j", label2);

            plabel(label);
            pinstr3("li", "$t0", "0");

            plabel(label2);
            pinstr3("sw", "$t0", res->id);

            free(label);
            free(label2);
            break;
    }
}

void qWrite (FILE *f, symbol *argv1) {
    char *label, *label2;

    switch (argv1->type) {
        case S_INT:
            snprintf(tbuf, LEN, "print integer %s", argv1->id);
            pcom(tbuf);

            pinstr3("li", "$v0", "1");
            pinstr3("lw", "$a0", argv1->id);
            break;

        case S_STRING:
            snprintf(tbuf, LEN, "print string %s", argv1->id);
            pcom(tbuf);

            pinstr3("li", "$v0", "4");
            pinstr3("la", "$a0", argv1->id);
            if(!argv1->tmp)
                pinstr3("move", "$a0", "$s0");
            break;

        case S_BOOL:
            label  = nextTmpLabel();
            label2 = nextTmpLabel();

            snprintf(tbuf, LEN, "print bool %s", argv1->id);
            pcom(tbuf);

            pinstr3("lw", "$t0", argv1->id);
            pinstr4("beq", "$t0", "$zero", label);
            pinstr3("la", "$a0", "_true");
            pinstr2("j", label2);

            plabel(label);
            pinstr3("la", "$a0", "_false");

            plabel(label2);
            pinstr3("li", "$v0", "4");
            break;
    }

    pinstr1("syscall");
}

void qArith (FILE *f, qop op, symbol *res, symbol *argv1, symbol *argv2) {
    snprintf(tbuf, LEN, "%s := %s %s %s", res->id, argv1->id, opstr(op), argv2->id);
    pcom(tbuf);

    pinstr3("lw", "$t0", argv1->id);
    pinstr3("lw", "$t1", argv2->id);

    char *label, *label2;
    switch (op) {
        case Q_PLUS  : pinstr4("add", "$t2", "$t0", "$t1") ; break ;
        case Q_MINUS : pinstr4("sub", "$t2", "$t0", "$t1") ; break ;
        case Q_MULT  : pinstr4("mul", "$t2", "$t0", "$t1") ; break ;
        case Q_DIV   : pinstr4("div", "$t2", "$t0", "$t1") ; break ;
        case Q_MOD   : pinstr4("rem", "$t2", "$t0", "$t1") ; break ;
        case Q_EXP   :
           label  = nextTmpLabel();
           label2 = nextTmpLabel();

           pinstr3("lw", "$t2", argv1->id);
           pinstr3("li", "$t3", "1");

           // warning: seulement les puissances > 0 fonctionnent avec ce code
           plabel(label);
           pinstr4("ble", "$t1", "$t3", label2);
           pinstr4("mul", "$t2", "$t2", "$t0");
           pinstr4("sub", "$t1", "$t1", "$t3");
           pinstr2("j", label);
           plabel(label2);

           free(label);
           free(label2);
           break;
    }

    pinstr3("sw", "$t2", res->id);
}

void qComp (FILE *f, qop op, symbol *res, symbol *argv1, symbol *argv2) {
    char *label, *label2;
    label  = nextTmpLabel();
    label2 = nextTmpLabel();

    snprintf(tbuf, LEN, ")%s := %s %s %s", res->id, argv1->id, opstr(op), argv2->id);
    pcom(tbuf);

    pinstr3("lw", "$t0", argv1->id);
    pinstr3("lw", "$t1", argv2->id);

    switch (op) {
        case Q_EQUAL:
            pinstr4("bne", "$t0", "$t1", label);
            break;
        case Q_DIFF:
            pinstr4("beq", "$t0", "$t1", label); break;
        case Q_INF:
            pinstr4("bge", "$t0", "$t1", label);
            break;
        case Q_INFEQ:
            pinstr4("bgt", "$t0", "$t1", label);
            break;
        case Q_SUP:
            pinstr4("ble", "$t0", "$t1", label);
            break;
        case Q_SUPEQ:
            pinstr4("blt", "$t0", "$t1", label);
            pinstr4("beq", "$t2", "$zero", label);
            break;
        case Q_OR:
            pinstr4("r", "$t2", "$t0", "$t1");
            pinstr4("beq", "$t2", "$zero", label);
            break;
        case Q_AND:
            pinstr4("and", "$t2", "$t0", "$t1");
            pinstr4("beq", "$t2", "$zero", label);
            break;
        case Q_XOR:
            pinstr4("xor", "$t2", "$t0", "$t1");
            pinstr4("beq", "$t2", "$zero", label);
            break;
        default:
            ferr("mips.c qComp wrong op");
    }

    pinstr3("li", "$t3", "1");
    pinstr2("j", label2);

    plabel(label);
    pinstr3("li", "$t3", "0");

    plabel(label2);
    pinstr3("sw", "$t3", res->id);

    free(label);
    free(label2);
}

void qNot (FILE *f, symbol *res, symbol *argv1) {
    char *label, *label2;
    label  = nextTmpLabel();
    label2 = nextTmpLabel();

    snprintf(tbuf, LEN, ")%s := NOT %s", res->id, argv1->id);
    pcom(tbuf);

    pinstr3("lw", "$t0", argv1->id);
    pinstr4("bne", "$t0", "$zero", label);
    pinstr3("li", "$t3", "1");
    pinstr2("j", label2);

    plabel(label);
    pinstr3("li", "$t3", "0");

    plabel(label2);
    pinstr3("sw", "$t3", res->id);

    free(label);
    free(label2);
}

///////////////
// FUNCTIONS //
///////////////

void fundec (FILE *f, symbol *fun) {
    snprintf(tbuf, LEN, "function %s", fun->id);
    pcom(tbuf);
    plabel(fun->id);

    // sauvegardage du ra
    pcom("push $ra to stack and load each arg from stack");

    pinstr4("sub", "$sp", "$sp", "4");
    pinstr3("sw", "$ra", "0($sp)");

    // load args from stack offset 4
    funStackLoadArgs(f, fun, 4);

    snprintf(tbuf, LEN, "body of function %s", fun->id);
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

    snprintf(tbuf, LEN, "epilogue of function %s", fun->id);
    pcom(tbuf);

    // label to jump to after a return
    snprintf(tbuf, LEN, "end_%s", fun->id);
    plabel(tbuf);

    // load saved ra to $ra
    pinstr3("lw", "$ra", "0($sp)");

    int offset = 4 + funArgsSize(fun);
    // pop ra and args from the stack
    snprintf(tbuf, LEN, "%d", offset);
    pinstr4("addi", "$sp", "$sp", tbuf);

    // jump to $ra
    pinstr2("jr", "$ra");
    snprintf(tbuf, LEN, "end of function %s", fun->id);
    pcom(tbuf);

    curfun = NULL;
}

void funcall (FILE *f, symbol *fun, symbol *args, symbol *res) {
    // caller push return adress + args to the stack if any, callee pop them before returning to $ra

    // show args string for debugging
    char argsDebug[LEN];
    funArgsDebugString(fun, argsDebug, LEN);

    if (res) {
        snprintf(tbuf, LEN, "funcall %s := %s ( %s )", res->id, fun->id, argsDebug);
        pcom(tbuf);
    } else {
        snprintf(tbuf, LEN, "funcall %s ( %s )", fun->id, argsDebug);
        pcom(tbuf);
    }

    int size;

    if (curfun != NULL) {
        pcom("push local vars to the stack");
        // make space for local var save in the stack
        size = curfunVarSize();
        snprintf(tbuf, LEN, "%d", size);
        pinstr4("sub", "$sp", "$sp", tbuf);

        // push local vars to the stack
        curfunStackPushVars(f);
    }

    pcom("push funcall args to the stack");
    // make space for args in the stack
    size = funArgsSize(fun);
    snprintf(tbuf, LEN, "%d", size);
    pinstr4("sub", "$sp", "$sp", tbuf);

    // push args to stack
    funStackPushArgs(f, args);

    /* Stack now (no curfun)    Stack now (curfun != NULL)
       0 -> arg 1               0 -> arg 1
       4 -> arg 2               4 -> arg 2
         etc ...                  etc ...
                                8  -> var1
                                12 -> var2
                                etc ...
    */

    // jump to function and put actual addr in $ra
    pinstr2("jal", fun->id);

    // store result ($v0) in res->id
    if (res)
        pinstr3("sw", "$v0", res->id);

    if (curfun != NULL) {
        /* Stack now (curfun != NULL)
           0 -> var1
           4 -> var2
           etc ...
       */

        pcom("load local vars from the stack");
        // load local vars from stack
        curfunStackLoadVars(f);
        size = curfunVarSize();

        // pop local vars from stack
        snprintf(tbuf, LEN, "%d", size);
        pinstr4("addi", "$sp", "$sp", tbuf);
    }
}

void funreturn (FILE *f, symbol *fun, symbol *ret) {
    if (ret) {
        snprintf(tbuf, LEN, "funreturn %s", ret->id);
        pcom(tbuf);
        pinstr3("lw", "$v0", ret->id);
    } else
        pcom("funreturn (void)");

    // goto function end label
    snprintf(tbuf, LEN, "end_%s", fun->id);
    pinstr2("j", tbuf);
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

    switch (sym->type) {
        case S_INT    : bytes = sizeof(int) ; break ;
        case S_BOOL   : bytes = sizeof(int) ; break ;
        default: ferr("mips.c funSymTypeSize arg wrong type");
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
        snprintf(tbuf, LEN, "%d($sp)", offset);
        pinstr3("lw", "$t0", tbuf);
        pinstr3("sw", "$t0", al->sym->id);

        bytes   = funSymTypeSize(al->sym);
        offset += bytes;
        al      = al->next;
    }
}

void funStackPushArgs (FILE *f, symbol *args) {
    int offset = 0, bytes;

    while (args != NULL) {
        pinstr3("lw", "$t0", args->id);
        snprintf(tbuf, LEN, "%d($sp)", offset);
        pinstr3("sw", "$t0", tbuf);

        bytes   = funSymTypeSize(args);
        offset += bytes;
        args    = args->next;
    }
}

void funArgsDebugString (symbol *fun, char *dstring, int maxlen) {
    int bytes, len = 0;
    arglist *al = ((fundata *) fun->fdata)->al;

    while (al != NULL) {
        bytes = snprintf(dstring + len, maxlen - len, "%s, ", al->sym->id);
        if (bytes < 0 || bytes >= LEN)
            ferr("mips.c funArgsDebugString snprintf");

        len += bytes;
        al = al->next;
    }

    if (len > 2)
        dstring[len - 2] = '\0'; // erase the last ", "
}

// curfun functions (only for function calls inside a function)

int curfunVarSize () {
    if (curfun == NULL)
        ferr("mips.c curfunVarSize - curfun is NULL");

    int size = 0, bytes;
    symbol *tos = ((fundata *) curfun->fdata)->tos;

    while (tos != NULL) {
        if (tos->type == S_INT || tos->type == S_BOOL) {
            bytes = funSymTypeSize(tos);
            size += bytes;
        }

        tos = tos->next;
    }

    return size;
}

void curfunStackPushVars (FILE *f) {
    if (curfun == NULL)
        ferr("mips.c curfunStackPushVars - curfun is NULL");

    int offset = 0, bytes;
    symbol *tos = ((fundata *) curfun->fdata)->tos;

    while (tos != NULL) {
        if (tos->type == S_INT || tos->type == S_BOOL) {
            pinstr3("lw", "$t0", tos->id);
            snprintf(tbuf, LEN, "%d($sp)", offset);
            pinstr3("sw", "$t0", tbuf);

            bytes = funSymTypeSize(tos);
            offset += bytes;
        }

        tos = tos->next;
    }
}

void curfunStackLoadVars (FILE *f) {
    if (curfun == NULL)
        ferr("mips.c curfunStackLoadVars - curfun is NULL");

    int offset = 0, bytes;
    symbol *tos = ((fundata *) curfun->fdata)->tos;

    while (tos != NULL) {
        if (tos->type == S_INT || tos->type == S_BOOL) {
            snprintf(tbuf, LEN, "%d($sp)", offset);
            pinstr3("lw", "$t0", tbuf);
            pinstr3("sw", "$t0", tos->id);

            bytes = funSymTypeSize(tos);
            offset += bytes;
        }

        tos = tos->next;
    }
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
 *  - l'appelant sauvegarde (empile) toutes les variables INT ou BOOL de sa table des symboles (tos) dans le stack (juste en-dessous des arguments de l'appel de fonction, avec la 1ere var locale juste après le dernier argument)
 *  - l'appelé ne touchera pas à ces variables locales sauvegardées
 *  - Une fois de retour juste après l'appel de fonction, l'appelant restaure ses variables locales depuis le stack et les dépiles de celui-ci.
 */

