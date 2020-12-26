#include "../headers/quad.h"
int nextquad = 0;

quad *qGen (qop op, symbol *res, symbol *argv1, symbol *argv2) {

    quad *nq = malloc(sizeof(struct quad));
    if (nq == NULL)
        ferr(__LINE__ ,"quad.c qGen malloc");

    nq->op    = op;
    nq->num   = nextquad ++;
    nq->next  = NULL;

    nq->res    = res;
    nq->argv1  = argv1;
    nq->argv2  = argv2;
    nq->gtrue  = NULL;
    nq->gfalse = NULL;
    nq->gnext  = NULL;

    return nq;
}

void qFree (quad *q) {
    quad *cur;

    while (q != NULL) {
        cur = q;
        q = q->next;
        free(cur);
    }
}

quad *qGet (quad *list, int num) {
    quad *q = list;
    while (q != NULL) {
        if (q->num == num)
            return q;
        q = q->next;
    }

    return NULL;
}

quad *concat (quad *q1, quad *q2) {
    quad *res = NULL;

    if (q1 != NULL) {
        res = q1;
        while (q1->next != NULL)
            q1 = q1->next;
        q1->next = q2;

    } else if (q2 != NULL)
        res = q2;

    return res;
}

void complete (quad *list, bool type, symbol *sym) {
    quad *cur = list;

    while (cur != NULL) {
        if (type)
            cur->gtrue = sym;
        else
            cur->gfalse = sym;

        cur = cur->next;
    }

    qFree(list);
}

quad *getLast (quad *q) {
    if (q == NULL)
        return NULL;

    while (q->next != NULL)
        q = q->next;

    return q;
}

void qPrint (quad *q) {
    quad *cur = q;
    if (cur == NULL)
        return;

    while (cur != NULL) {
        switch (cur->op) {
            case Q_PLUS:
                fprintf(stdout, "ADD\t\t");
                break;
            case Q_MINUS:
                fprintf(stdout, "SUB\t\t");
                break;
            case Q_MULT:
                fprintf(stdout, "MULT\t\t");
                break;
            case Q_DIV:
                fprintf(stdout, "DIV\t\t");
                break;
            case Q_EXP:
                fprintf(stdout, "EXP\t\t");
                break;
            case Q_WRITE:
                fprintf(stdout, "WRITE\t\t");
                break;
            case Q_AFFEC:
                fprintf(stdout, "AFFEC\t\t");
                break;
            default:
                //TO AVOID WARNINGS AT COMPILATION MAYBE TODO ?
                break;
        }

        if (cur->argv1 == NULL)
            fprintf(stdout, "%s\t\t", "NULL");
        else
            fprintf(stdout, "%s\t\t", cur->argv1->id);

        if (cur->argv2 == NULL)
            fprintf(stdout, "%s\t\t", "NULL");
        else
            fprintf(stdout, "%s\t\t", cur->argv2->id);

        if (cur->res == NULL)
            fprintf(stdout, "%s\t\t", "NULL");
        else
            fprintf(stdout, "%s\t\t", cur->res->id);

        fprintf(stdout, "\n");
        cur = cur->next;
    }
}
