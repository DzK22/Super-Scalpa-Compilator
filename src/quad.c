#include "../headers/quad.h"

quad *qGen (qop op, symbol *res, symbol *argv1, symbol *argv2) {
    static int nquad = 0;
    quad *nq = malloc(sizeof(struct quad));
    if (nq == NULL) {
        fprintf(stderr, "malloc error\n");
        return NULL;
    }
    nq->op = op;
    nq->res = res;
    nq->argv1 = argv1;
    nq->argv2 = argv2;
    nq->num = nquad++;
    nq->nlab = false;
    nq->next = NULL;
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

quad *concat (quad *q1, quad *q2) {
    quad *res = NULL;
    if (q1 != NULL) {
        res = q1;
        while (q1->next != NULL)
            q1 = q1->next;
        q1->next = q2;
    }
    else if (q2 != NULL)
        res = q2;
    return res;
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
            default:
                break;
        }
        if (cur->argv1 == NULL)
            fprintf(stdout, "%10s\t\t", "NULL");
        else {
            if (cur->argv1->cst)
                fprintf(stdout, "%10d\t\t", cur->argv1->value);
            else
                fprintf(stdout, "%10s\t\t", cur->argv1->string);
        }

        if (cur->argv2 == NULL)
            fprintf(stdout, "%10s\t\t", "NULL");
        else {
            if (cur->argv2->cst)
                fprintf(stdout, "%10d\t\t", cur->argv2->value);
            else
                fprintf(stdout, "%10s\t\t", cur->argv2->string);
        }

        if (cur->res == NULL)
            fprintf(stdout, "%10s\t\t", "NULL");
        else {
            if (cur->res->cst)
                fprintf(stdout, "%10d\t\t", cur->res->value);
            else
                fprintf(stdout, "%10s\t\t", cur->res->id);
        }

        fprintf(stdout, "\n");
        cur = cur->next;
    }
}
