#include "../headers/mips.h"

void getMips (FILE *f, symbol *s, quad *q) {
    symbol *res, *argv1, *argv2;
    fprintf(f, "\t.data\n");
    while (s != NULL) {
        if (s->cst) {
            switch (s->type) {
                case INTEGER_:
                    fprintf(f, "%s:\t.word %d\n", s->id, s->value);
                    break;
                default:
                    break;
            }
        }
        else
            fprintf(f, "%s:\t.word 0\n", s->id);
        s = s->next;
    }
    fprintf(f, "\t.text\n\t.globl main\n");
    fprintf(f, "main: ");
    while (q != NULL) {
        res = q->res;
        argv1 = q->argv1;
        argv2 = q->argv2;
        switch (q->op) {
            case Q_PLUS:
                fprintf(stdout, "%s = %s + %s\n", res->id, argv1->id, argv2->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tlw $t1, %s\n", argv2->id);
                fprintf(f, "\tadd $t2, $t0, $t1\n");
                break;

            case Q_MINUS:
                fprintf(stdout, "%s = %s - %s\n", res->id, argv1->id, argv2->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tlw $t1, %s\n", argv2->id);
                fprintf(f, "\tsub $t2, $t0, $t1\n");
                break;

            case Q_MULT:
                fprintf(stdout, "%s = %s * %s\n", res->id, argv1->id, argv2->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tlw $t1, %s\n", argv2->id);
                fprintf(f, "\tmult $t2, $t0, $t1\n");
                break;

            default:
                fprintf(stderr, "unknown\n");
                break;
        }

        if (q->op == Q_PLUS || q->op == Q_MINUS || q->op == Q_MULT || q->op == Q_DIV || q->op == Q_EXP)
            fprintf(f, "\tsw $t2, %s\n", res->id);
        q = q->next;
    }

    //quitter le programme proprement
    fprintf(f, "exit: \n");
    fprintf(f, "\tli $v0, 10\n");
    fprintf(f, "\tsyscall\n");
}
