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
            case S_INTEGER:
                fprintf(f, "%s:\t.word %d\n", s->id, s->val);
                break;
            case S_STRING:
                fprintf(f, "%s:\t.ascii %s\n", s->id, s->str);
                break;
        }

        s = s->next;
    }
}

void getText (FILE *f, quad *q) {
    symbol *res, *argv1, *argv2;

    while (q != NULL) {
        res = q->res;
        argv1 = q->argv1;
        argv2 = q->argv2;

        switch (q->op) {
            case Q_PLUS:
                fprintf(f, "\t\t\t\t#%s = %s + %s\n", res->id, argv1->id, argv2->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tlw $t1, %s\n", argv2->id);
                fprintf(f, "\tadd $t2, $t0, $t1\n");
                fprintf(f, "\tsw $t2, %s\n", res->id);
                break;

            case Q_MINUS:
                fprintf(f, "\t\t\t\t#%s = %s - %s\n", res->id, argv1->id, argv2->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tlw $t1, %s\n", argv2->id);
                fprintf(f, "\tsub $t2, $t0, $t1\n");
                fprintf(f, "\tsw $t2, %s\n", res->id);
                break;

            case Q_MULT:
                fprintf(f, "\t\t\t\t#%s = %s * %s\n", res->id, argv1->id, argv2->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tlw $t1, %s\n", argv2->id);
                fprintf(f, "\tmult $t2, $t0, $t1\n");
                fprintf(f, "\tsw $t2, %s\n", res->id);
                break;

            case Q_WRITE:
                fprintf(f, "\t\t\t\t#print integer %s\n", res->id);
                fprintf(f, "\tlw $a0, %s\n", res->id);
                fprintf(f, "\tli $v0, 1\n");
                fprintf(f, "\tsyscall\n");
                break;

            case Q_AFFEC:
                fprintf(f, "\t\t\t\t#%s := %s\n", res->id, argv1->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tsw $t0, %s\n", res->id);
                break;

            default:
                ferr("mips.c getText unknown op");
        }

        q = q->next;
    }
}
