#ifndef QUAD_H
#define QUAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "stable.h"

typedef enum {
  Q_END, Q_WRITE, Q_READ, Q_AFFEC,       // divers
  Q_PLUS, Q_MINUS, Q_MULT, Q_DIV, Q_EXP, Q_INF, Q_INFEQ, Q_SUP, Q_SUPEQ, Q_EQUAL, Q_DIFF, Q_AND, Q_OR, Q_XOR, Q_NOT, // operators (binary or unary)
  Q_LABEL, Q_GOTO, Q_IF, Q_IFELSE
} qop;

typedef struct gotoLst gotoLst;

typedef struct quad {
    qop             op;
    int             num;
    struct quad     *next;

    symbol          *res;
    symbol          *argv1;
    symbol          *argv2;

    symbol          *gtrue;
    symbol          *gfalse;
    symbol          *gnext;
} quad;

quad *qGen     (qop, symbol *, symbol *, symbol *);
void qFree     (quad *);
quad *concat   (quad *, quad *);
quad *qGet     (quad *, int);
quad *getLast  (quad *);
void qPrint    (quad *);
void ferr      (char *s);
void complete  (quad *, bool, symbol *);

#endif
