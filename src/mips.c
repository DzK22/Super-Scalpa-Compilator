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

void getText (FILE *f, quad *q) {
    symbol *res, *argv1, *argv2;

    while (q != NULL) {
        res = q->res;
        argv1 = q->argv1;
        argv2 = q->argv2;

        switch (q->op) {
            case Q_PLUS:
                if (!res || !argv1 || !argv2)
                    ferr("mips.c getText Q_PLUS Quad error");

                fprintf(f, "\t\t\t\t#%s = %s + %s\n", res->id, argv1->id, argv2->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tlw $t1, %s\n", argv2->id);
                fprintf(f, "\tadd $t2, $t0, $t1\n");
                fprintf(f, "\tsw $t2, %s\n", res->id);
                break;

            case Q_MINUS:
                if (!res || !argv1 || !argv2)
                    ferr("mips.c getText Q_MINUS Quad error");

                fprintf(f, "\t\t\t\t#%s = %s - %s\n", res->id, argv1->id, argv2->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tlw $t1, %s\n", argv2->id);
                fprintf(f, "\tsub $t2, $t0, $t1\n");
                fprintf(f, "\tsw $t2, %s\n", res->id);
                break;

            case Q_MULT:
                if (!res || !argv1 || !argv2)
                    ferr("mips.c getText Q_MULT Quad error");

                fprintf(f, "\t\t\t\t#%s = %s * %s\n", res->id, argv1->id, argv2->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tlw $t1, %s\n", argv2->id);
                fprintf(f, "\tmul $t2, $t0, $t1\n");
                fprintf(f, "\tsw $t2, %s\n", res->id);
                break;

            case Q_WRITE:
                if (!res)
                    ferr("mips.c getText Q_WRITE Quad error");

                switch (res->type) {
                    case S_INT:
                        fprintf(f, "\t\t\t\t#print integer %s\n", res->id);
                        fprintf(f, "\tlw $a0, %s\n", res->id);
                        fprintf(f, "\tli $v0, 1\n");
                        break;

                    case S_STRING:
                        fprintf(f, "\t\t\t\t#print string %s\n", res->id);
                        fprintf(f, "\tli $v0, 4\n");
                        fprintf(f, "\tla $a0, %s\n", res->id);
                        break;
                }
                fprintf(f, "\tsyscall\n");
                break;

            case Q_AFFEC:
                if (!res || !argv1)
                    ferr("mips.c getText Q_AFFEC Quad error");

                fprintf(f, "\t\t\t\t#%s := %s\n", res->id, argv1->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tsw $t0, %s\n", res->id);
                break;

            case Q_LABEL:
                fprintf(f, "%s:\n", res->id);
                break;

            case Q_GOTO:
                fprintf(f, "\tj %s\n", res->id);
                break;

            case Q_EQUAL:
                fprintf(f, "\t\t\t\t#goto %s if %s == %s\n", res->id, argv1->id, argv2->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tlw $t1, %s\n", argv2->id);
                fprintf(f, "\tbeq $t0, $t1, %s\n", res->id);
                break;

            case Q_INF:
                fprintf(f, "\t\t\t\t#goto %s if %s < %s\n", res->id, argv1->id, argv2->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tlw $t1, %s\n", argv2->id);
                fprintf(f, "\tblt $t0, $t1, %s\n", res->id);
                break;

            case Q_INFEQ:
                fprintf(f, "\t\t\t\t#goto %s if %s <= %s\n", res->id, argv1->id, argv2->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tlw $t1, %s\n", argv2->id);
                fprintf(f, "\tble $t0, $t1, %s\n", res->id);
                break;

            case Q_SUP:
                fprintf(f, "\t\t\t\t#goto %s if %s > %s\n", res->id, argv1->id, argv2->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tlw $t1, %s\n", argv2->id);
                fprintf(f, "\tbgt $t0, $t1, %s\n", res->id);
                break;

            case Q_SUPEQ:
                fprintf(f, "\t\t\t\t#goto %s if %s >= %s\n", res->id, argv1->id, argv2->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tlw $t1, %s\n", argv2->id);
                fprintf(f, "\tbge $t0, $t1, %s\n", res->id);
                break;

            case Q_DIFF:
                fprintf(f, "\t\t\t\t#goto %s if %s != %s\n", res->id, argv1->id, argv2->id);
                fprintf(f, "\tlw $t0, %s\n", argv1->id);
                fprintf(f, "\tlw $t1, %s\n", argv2->id);
                fprintf(f, "\tbne $t0, $t1, %s\n", res->id);
                break;

            default:
                ferr("mips.c getText unknown op");
        }

        q = q->next;
    }
}
