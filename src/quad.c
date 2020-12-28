#include "../headers/quad.h"

quad *qGen (qop op, symbol *res, symbol *argv1, symbol *argv2) {
    quad *nq = malloc(sizeof(struct quad));
    if (nq == NULL)
        ferr(__LINE__ ,"quad.c qGen malloc");

    nq->op     = op;
    nq->next   = NULL;
    nq->gfalse = NULL;

    nq->res    = res;
    nq->argv1  = argv1;
    nq->argv2  = argv2;

    return nq;
}

void qFree (quad *q) {
    quad *cur;

    while (q != NULL) {
        cur = q;
        q   = q->next;
        free(cur);
    }
}

quad *qConcat (quad *q1, quad *q2) {
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

void qPrint (quad *q) {
    quad *cur = q;
    if (cur == NULL)
        return;

    while (cur != NULL) {
        switch (cur->op) {
            case Q_PLUS:
                printf("ADD\t\t");
                break;
            case Q_MINUS:
                printf("SUB\t\t");
                break;
            case Q_MULT:
                printf("DIV\t\t");
                break;
            case Q_EXP:
                printf("WRITE\t\t");
                break;
            case Q_AFFEC:
                printf("AFFEC\t\t");
                break;
            default:
                break;
        }

        if (cur->argv1 == NULL)
            printf("%s\t\t", "NULL");
        else
            printf("%s\t\t", cur->argv1->id);

        if (cur->argv2 == NULL)
            printf("%s\t\t", "NULL");
        else
            printf("%s\t\t", cur->argv2->id);

        if (cur->res == NULL)
            printf("%s\t\t", "NULL");
        else
            printf("%s\t\t", cur->res->id);

        printf("\n");
        cur = cur->next;
    }
}
