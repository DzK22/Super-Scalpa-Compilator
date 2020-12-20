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
    fprintf(f, "\n\t.text\n\t.globl main\nmain:\n");
    getText(f, q);

    // quitter le programme proprement
    fprintf(f, "\nexit:\n");
    fprintf(f, "\tli $v0, 10\n");
    fprintf(f, "\tsyscall\n");
}

void getData (FILE *f, symbol *s) {
    while (s != NULL) {
        switch (s->type) {
            case S_INT:
                fprintf(f, "%s:\t.word %d\n", s->id, s->ival);
                break;
            case S_STRING:
                fprintf(f, "%s:\t.ascii %s\n", s->id, s->sval);
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
                if (!res)
                    ferr("mips.c getText Q_WRITE Quad error");

                switch (res->type) {
                    case S_INT:
                        fprintf(f, "\t\t\t\t# print integer %s\n", res->id);
                        fprintf(f, "\tlw $a0, %s\n", res->id);
                        fprintf(f, "\tli $v0, 1\n");
                        break;

                    case S_STRING:
                        fprintf(f, "\t\t\t\t# print string %s\n", res->id);
                        fprintf(f, "\tli $v0, 4\n");
                        fprintf(f, "\tla $a0, %s\n", res->id);
                        break;
                }
                fprintf(f, "\tsyscall\n");
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
                    case Q_AND   : fprintf(f, "\tand $t2, $t0, $t1");
                                   fprintf(f, "\tbeq $t2, $zero, %s\n", label);
                                   break;
                    case Q_OR    : fprintf(f, "\tor $t2, $t0, $t1");
                                   fprintf(f, "\tbeq $t2, $zero, %s\n", label);
                                   break;
                    case Q_XOR   : fprintf(f, "\txor $t2, $t0, $t1");
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

            default:
                ferr("mips.c getText unknown op");
        }

        q = q->next;
    }
}
