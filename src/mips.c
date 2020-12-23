#include "../headers/mips.h"

void ferr (char *s) {
    fprintf(stderr, "Error: %s\n", s);
    exit(EXIT_FAILURE);
}

void getMips (FILE *f, symbol *s, quad *q) {
    // data
    fprintf(f, "\t.data\n");
    getData(f, s);

    // text
    fprintf(f, "\n\t.text\n\t.globl main\n");
    getText(f, q);

    // quitter le programme proprement
    fprintf(f, "\nexit:\n");
    fprintf(f, "\tli $v0, 10\n");
    fprintf(f, "\tsyscall\n");
}

void getData (FILE *f, symbol *s) {
    // initial
    fprintf(f, "_true:\t.asciiz \"true\"\n");
    fprintf(f, "_false:\t.asciiz \"false\"\n");
    fprintf(f, "_read_int: .asciiz \"Enter int: \"\n");
    fprintf(f, "_read_string: .asciiz \"Enter string: \"\n");
    fprintf(f, ".align 2\n");
    fprintf(f, "_buffer: .space %d\n", MIPS_BUFFER_SPACE);

    while (s != NULL) {
        switch (s->type) {
            case S_INT:
                fprintf(f, "%s:\t.word %d\n", s->id, s->ival);
                break;
            case S_BOOL:
                fprintf(f, "%s:\t.word %d\n", s->id, s->bval);
                break;
            case S_STRING:
                fprintf(f, "%s:\t.asciiz %s\n", s->id, s->sval);
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
    char *label, *label2;
    fundata *fdata;
    arglist *al;

    int offset, bytes, len;
    symbol *sym;

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
            case Q_EXP:
                if (!res || !argv1 || !argv2)
                    ferr("mips.c getText arith quad error");
                fprintf(f, "\t\t\t\t# %s = %s %s %s\n", res->id, argv1->id, opstr(q->op), argv2->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tlw $t1, %s\n", argv2->id);

                switch (q->op) {
                    case Q_PLUS  : fprintf(f, "\tadd $t2, $t0, $t1\n") ; break ;
                    case Q_MINUS : fprintf(f, "\tsub $t2, $t0, $t1\n") ; break ;
                    case Q_MULT  : fprintf(f, "\tmul $t2, $t0, $t1\n") ; break ;
                    case Q_DIV   : fprintf(f, "\tdiv $t2, $t0, $t1\n") ; break ;
                    case Q_EXP   :
                        label  = nextTmpLabel();
                        label2 = nextTmpLabel();

                        fprintf(f, "\tlw $t2, %s\n", argv1->id);
                        fprintf(f, "\tli $t3, 1\n");
                        fprintf(f, "\n%s:\n", label);
                        fprintf(f, "\tble $t1, $t3, %s\n", label2);
                        fprintf(f, "\tmul $t2, $t2, $t0\n");
                        fprintf(f, "\tsub $t1, $t1, $t3\n");
                        fprintf(f, "\tj %s\n", label);
                        fprintf(f, "\n%s:\n", label2);

                        free(label);
                        free(label2);
                        break;
                        // warning: seulement les puissances > 0 fonctionnent avec ce code
                }

                fprintf(f, "\tsw $t2, %s\n", res->id);
                break;

            case Q_WRITE:
                if (!argv1)
                    ferr("mips.c getText Q_WRITE quad error");

                switch (argv1->type) {
                    case S_INT:
                        fprintf(f, "\t\t\t\t# print integer %s\n", argv1->id);
                        fprintf(f, "\tli $v0, 1\n");
                        fprintf(f, "\tlw $a0, %s\n", argv1->id);
                        break;

                    case S_STRING:
                        fprintf(f, "\t\t\t\t# print string %s\n", argv1->id);
                        fprintf(f, "\tli $v0, 4\n");
                        fprintf(f, "\tla $a0, %s\n", argv1->id);
                        if(!argv1->tmp)
                            fprintf(f, "\tmove $a0, $s0\n");
                        break;

                    case S_BOOL:
                        label  = nextTmpLabel();
                        label2 = nextTmpLabel();

                        fprintf(f, "\t\t\t\t# print bool %s\n", argv1->id);
                        fprintf(f, "\tlw $t0, %s\n", argv1->id);
                        fprintf(f, "\tbeq $t0, $zero, %s\n", label);
                        fprintf(f, "\tla $a0, _true\n");
                        fprintf(f, "\tj %s\n", label2);
                        fprintf(f, "\n%s:\n", label);
                        fprintf(f, "\tla $a0, _false\n");
                        fprintf(f, "\n%s:\n", label2);
                        fprintf(f, "\tli $v0, 4\n");
                        break;
                }

                fprintf(f, "\tsyscall\n");
                break;

            case Q_READ:
                if (!res)
                    ferr("mips.c getText Q_READ quad error");

                switch (res->type) {
                    case S_INT:
                        fprintf(f, "\t\t\t\t# read integer %s\n", res->id);
                        fprintf(f, "\tli $v0, 4\n");
                        fprintf(f, "\tla $a0, _read_int\n");
                        fprintf(f, "\tsyscall\n");
                        fprintf(f, "\tli $v0, 5\n");
                        fprintf(f, "\tsyscall\n");
                        fprintf(f, "\tsw $v0, %s\n", res->id);
                        break;

                    case S_STRING:
                        fprintf(f, "\t\t\t\t# read string %s\n", res->id);
                        fprintf(f, "\tli $v0, 4\n");
                        fprintf(f, "\tla $a0, _read_string\n");
                        fprintf(f, "\tsyscall\n");
                        fprintf(f, "\tli $v0, 8\n");
                        fprintf(f, "\tla $a0, _buffer\n");
                        fprintf(f, "\tli $a1, %d\n", MIPS_BUFFER_SPACE);
                        fprintf(f, "\tmove $s0, $a0\n");
                        fprintf(f, "\tsyscall\n");
                        break;

                    case S_BOOL:
                        label  = nextTmpLabel();
                        label2 = nextTmpLabel();

                        fprintf(f, "\t\t\t\t# read bool %s\n", res->id);
                        fprintf(f, "\tli $v0, 4\n");
                        fprintf(f, "\tla $a0, _read_int\n");
                        fprintf(f, "\tsyscall\n");
                        fprintf(f, "\tli $v0, 5\n");
                        fprintf(f, "\tsyscall\n");
                        fprintf(f, "\tbeq $v0, $zero, %s\n", label);
                        fprintf(f, "\tli $t0, 1\n");
                        fprintf(f, "\tj %s\n", label2);
                        fprintf(f, "\n%s:\n", label);
                        fprintf(f, "\tli $t0, 0\n");
                        fprintf(f, "\n%s:\n", label2);
                        fprintf(f, "\nsw $t0, %s\n", res->id);

                        free(label);
                        free(label2);
                        break;
                }

                break;

            case Q_AFFEC:
                if (!res || !argv1)
                    ferr("mips.c getText Q_AFFEC quad error");

                fprintf(f, "\t\t\t\t# %s := %s\n", res->id, argv1->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tsw $t0, %s\n", res->id);
                break;

            case Q_LABEL:
                if (!res)
                    ferr("mips.c getText Q_LABEL quad error");

                fprintf(f, "\n%s:\n", res->id);
                break;

            case Q_GOTO:
                if (!res)
                    ferr("mips.c getText Q_GOTO quad error");

                fprintf(f, "\t\t\t\t# goto %s\n", res->id);
                fprintf(f, "\tj %s\n", res->id);
                break;

            case Q_IF:
                if (!argv1 || !gtrue || !gfalse || !gnext)
                    ferr("mips.c getText Q_IF quad error");

                fprintf(f, "\t\t\t\t# if %s is false then goto %s\n", argv1->id, gfalse->sval);

                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tbeq $t0, $zero, %s\n", gfalse->sval);
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

                label  = nextTmpLabel();
                label2 = nextTmpLabel();

                fprintf(f, "\t\t\t\t# %s := %s %s %s\n", res->id, argv1->id, opstr(q->op), argv2->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tlw $t1, %s\n", argv2->id);

                switch (q->op) {
                    case Q_EQUAL : fprintf(f, "\tbne $t0, $t1, %s\n", label); break;
                    case Q_DIFF  : fprintf(f, "\tbeq $t0, $t1, %s\n", label); break;
                    case Q_INF   : fprintf(f, "\tbge $t0, $t1, %s\n", label); break;
                    case Q_INFEQ : fprintf(f, "\tbgt $t0, $t1, %s\n", label); break;
                    case Q_SUP   : fprintf(f, "\tble $t0, $t1, %s\n", label); break;
                    case Q_SUPEQ : fprintf(f, "\tblt $t0, $t1, %s\n", label); break;
                                   fprintf(f, "\tbeq $t2, $zero, %s\n", label);
                                   break;
                    case Q_OR    : fprintf(f, "\tor $t2, $t0, $t1\n");
                                   fprintf(f, "\tbeq $t2, $zero, %s\n", label);
                                   break;
                    case Q_AND   : fprintf(f, "\tand $t2, $t0, $t1\n");
                                   fprintf(f, "\tbeq $t2, $zero, %s\n", label);
                                   break;
                    case Q_XOR   : fprintf(f, "\txor $t2, $t0, $t1\n");
                                   fprintf(f, "\tbeq $t2, $zero, %s\n", label);
                                   break;

                }

                fprintf(f, "\tli $t3 1\n");
                fprintf(f, "\tj %s\n", label2);
                fprintf(f, "\n%s:\n", label);
                fprintf(f, "\tli $t3 0\n");
                fprintf(f, "\n%s:\n", label2);
                fprintf(f, "\tsw $t3 %s\n", res->id);

                free(label);
                free(label2);
                break;

            case Q_NOT:
                if (!res || !argv1)
                    ferr("mips.c getText Q_NOT quad error");

                label  = nextTmpLabel();
                label2 = nextTmpLabel();

                fprintf(f, "\t\t\t\t# %s := NOT %s\n", res->id, argv1->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tbne $t0, $zero, %s\n", label);
                fprintf(f, "\tli $t3 1\n");
                fprintf(f, "\tj %s\n", label2);
                fprintf(f, "\n%s:\n", label);
                fprintf(f, "\tli $t3 0\n");
                fprintf(f, "\n%s:\n", label2);
                fprintf(f, "\tsw $t3 %s\n", res->id);

                free(label);
                free(label2);
                break;

            case Q_FUNDEC:
                if (!argv1)
                    ferr("mips.c getText Q_FUNDEC quad error");
                // argv1 = function symbol

                fprintf(f, "\n\t\t\t\t# function %s\n", argv1->id);
                fprintf(f, "%s:\n", argv1->id);

                fdata  = (fundata *) argv1->fdata;
                offset = 0;
                al = fdata->al;

                // sauvegardage du ra
                fprintf(f, "\t\t\t\t# on empile $ra et récupère chaque argument de la pile\n");
                fprintf(f, "\tsub $sp, $sp, 4\n");
                fprintf(f, "\tsw $ra, %d($sp)\n", offset);
                offset += 4;

                /* pile a ce moment:
                  0  -> ra
                  -4 -> arg 1
                  -8 -> arg 2
                  etc ...
                */

                while (al != NULL) {
                  sym = al->sym;

                  switch (sym->type) {
                      case S_INT  : bytes = 4; break;
                      case S_BOOL : bytes = 4; break;
                      default: ferr("mips.c getText Q_FUNDEC wrong arg type");
                  }

                  fprintf(f, "\tlw $t0, %d($sp)\n", offset);
                  fprintf(f, "\tsw $t0, %s\n", sym->id);
                  offset += bytes;
                  al = al->next;
                }

                fprintf(f, "\t\t\t\t# body of function %s\n\n", argv1->id);
                break;

            case Q_FUNEND: // end of declaration of a function
                if (!argv1)
                    ferr("mips.c getText Q_FUNEND quad error");
                // argv1 = function symbol

                // pop args from the stack and put sav ra in $ra
                fprintf(f, "\n\t\t\t\t# epilogue of function %s\n", argv1->id);
                // label où jumper après un return
                fprintf(f, "end_%s:\n", argv1->id);

                /* pile a ce moment:
                  0  -> ra
                  -4 -> arg n
                  -8 -> arg n - 1
                  etc ...
                */

                fdata = (fundata *) argv1->fdata;
                al = fdata->al;
                offset = 0;
                fprintf(f, "\tlw $ra, %d($sp)\n", offset);
                offset += 4;

                while (al != NULL) {
                  sym = al->sym;

                  switch (sym->type) {
                      case S_INT  : bytes = 4; break;
                      case S_BOOL : bytes = 4; break;
                      default: ferr("mips.c getText Q_FUNDEC wrong arg type");
                  }

                  offset += bytes;
                  al = al->next;
                }

                fprintf(f, "\taddi $sp, $sp, %d\n", offset);
                fprintf(f, "\tjr $ra\n");
                fprintf(f, "\t\t\t\t# end of function %s\n", argv1->id);
                break;

            case Q_FUNCALL:
                // res can be NULL = fun type S_UNIT
                // argv2 can be NULL = no args
                if (!argv1)
                    ferr("mips.c getText Q_FUNCALL quad error");
                // res   = where to put function return value
                // arvg1 = function symbol
                // argv2 = list of symbol = symbol * (!= stable)

                // caller push return adress + args to the stack if any, callee pop them before returning to $ra

                // args str for debugging
                char argsDebug[LEN] = "";
                len    = 0;
                offset = 0;
                sym    = argv2;

                // debug args string + calcul du total offset
                while (sym != NULL) {
                    bytes = snprintf(argsDebug + len, LEN - len, "%s, ", sym->id);
                    if (bytes < 0 || bytes >= LEN)
                        ferr("mips.c getText Q_FUNCALL snprintf");

                    len += bytes;
                    switch (sym->type) {
                      case S_INT  : bytes = 4; break;
                      case S_BOOL : bytes = 4; break;
                      default: ferr("mips.c getText Q_FUNCALL wrong arg type");
                    }

                    offset += bytes;
                    sym = sym->next;
                }

                // print debug args
                if (len > 2)
                    argsDebug[len - 2] = '\0'; // erase the last ", "

                // empiler chaque arg
                fprintf(f, "\tsub $sp, $sp, %d\n", offset);
                sym = argv2;
                offset = 0;

                while (sym != NULL) {
                    // push args
                    switch (sym->type) {
                      case S_INT  : bytes = 4; break;
                      case S_BOOL : bytes = 4; break;
                      default: ferr("mips.c getText Q_FUNCALL wrong arg type");
                    }

                    fprintf(f, "\t\t\t\t # on pousse larg %s\n", sym->id);
                    fprintf(f, "\tlw $t0, %s\n", sym->id);
                    fprintf(f, "\tsw $t0, %d($sp)\n", offset);

                    offset += bytes;
                    sym = sym->next;
                }

                // afficher debug mess
                if (res)
                    fprintf(f, "\t\t\t\t# funcall %s := %s ( %s )\n", res->id, argv1->id, argsDebug);
                else
                    fprintf(f, "\t\t\t\t# funcall %s ( %s )\n", argv1->id, argsDebug);

                // jump to function and put actual addr in $ra
                fprintf(f, "\tjal %s\n", argv1->id);

                // store result ($v0) in res->id
                if (res)
                    fprintf(f, "\tsw $v0, %s\n", res->id);
                break;

            case Q_FUNRETURN:
                if (!argv1) // argv2 optional
                    ferr("mips.c getText Q_FUNRETURN quad error");
                // arvg1 = function symbol
                // arvg2 = symbol to return (can be NULL if fun unit)

                if (argv2) {
                    fprintf(f, "\t\t\t\t# funreturn %s\n", argv2->id);
                    fprintf(f, "\tlw $v0, %s\n", argv2->id);
                } else
                    fprintf(f, "\t\t\t\t# funreturn (void)\n");

                // goto function end label
                fprintf(f, "\tj end_%s\n", argv1->id);
                break;

            case Q_MAIN:
                fprintf(f, "\n\t\t\t\t# main function\n");
                fprintf(f, "main:\n");
                break;

            default:
                ferr("mips.c getText unknown op");
        }

        q = q->next;
    }
}
