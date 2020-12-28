#ifndef QUAD_H
#define QUAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "stable.h"
#include "types.h"
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
