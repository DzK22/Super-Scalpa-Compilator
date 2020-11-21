#ifndef QUAD_H
#define QUAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "stable.h"

typedef enum {Q_PLUS, Q_MINUS, Q_MULT, Q_DIV, Q_EXP} qop;

typedef struct quad {
    qop             op;
    symbol          *argv1;
    symbol          *argv2;
    symbol          *res;
    struct quad     *next;
} quad;

quad *qGen (qop, symbol *, symbol *, symbol *);
void qFree (quad *);
quad *concat (quad *, quad *);
void qPrint (quad *q);
#endif
