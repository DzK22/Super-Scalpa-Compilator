#ifndef QUAD_H
#define QUAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "stable.h"

typedef enum {
  Q_PLUS, Q_MINUS, Q_MULT, Q_DIV, Q_EXP, Q_INF, Q_INFEQ, Q_SUP, Q_SUPEQ, Q_EQUAL, Q_DIFF, Q_AND, Q_OR, Q_XOR, Q_NOT, Q_MOD,// operators (binary or unary)
  Q_FUNDEC, Q_FUNEND, Q_FUNCALL, Q_FUNRETURN, // for functions
  Q_END, Q_WRITE, Q_READ, Q_AFFEC, Q_LABEL, Q_GOTO, Q_IF, Q_IFELSE, Q_MAIN // other
} qop;

typedef struct quad {
    qop          op;
    int          num;
    struct quad  *next;

    struct symbol *res;
    struct symbol *argv1;
    struct symbol *argv2;

    struct symbol *gtrue;
    struct symbol *gfalse;
    struct symbol *gnext;
} quad;

quad *qGen     (qop, struct symbol *, struct symbol *, struct symbol *);
void qFree     (quad *);
quad *concat   (quad *, quad *);
quad *qGet     (quad *, int);
quad *getLast  (quad *);
void qPrint    (quad *);
void ferr      (int , char *s);
void complete  (quad *, bool, struct symbol *);

#endif
