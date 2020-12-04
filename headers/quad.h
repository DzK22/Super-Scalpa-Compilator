#ifndef QUAD_H
#define QUAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "stable.h"

typedef enum {Q_PLUS, Q_MINUS, Q_MULT, Q_DIV, Q_EXP, Q_END, Q_WRITE, Q_AFFEC} qop;

typedef struct quad {
    qop             op;
    int             num;
    bool            nlab;
    symbol          *res;
    symbol          *argv1;
    symbol          *argv2;
    struct quad     *next;
} quad;

quad *qGen (qop, symbol *, symbol *, symbol *);
void qFree (quad *);
quad *concat (quad *, quad *);
quad *getLast (quad *);
void qPrint (quad *);
void ferr (char *s);

#endif
