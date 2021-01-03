#ifndef OPTI_H
#define OPTI_H

#include "util.h"
#include "quad.h"
#include "stable.h"
#include "list.h"

void optiLoop          (quad **code, symbol **gtos);
int optiDeadCode       (quad **code, symbol **tos);
int optiDuplicateCst   (quad **code, symbol **tos);
bool optiCheckModified (quad *code, symbol *s);
bool sameTypeValue     (symbol *s1, symbol *s2);
int optiArithOp        (quad **code, symbol **tos);
int zeroAdd            (quad *q);
int oneMult            (quad *q);

#endif
