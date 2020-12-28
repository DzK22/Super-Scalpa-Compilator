#ifndef QUAD_H
#define QUAD_H

#include "util.h"
#include "stable.h"
#include "types.h"

typedef struct quad {
    qop           op;
    struct quad   *next;
    struct symbol *gfalse;

    struct symbol *res;
    struct symbol *argv1;
    struct symbol *argv2;
} quad;

quad *qGen    (qop, struct symbol *, struct symbol *, struct symbol *);
void  qFree   (quad *);
quad *qConcat (quad *, quad *);
void  qPrint  (quad *);

#endif
