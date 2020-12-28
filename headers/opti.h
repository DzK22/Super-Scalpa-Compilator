#ifndef OPTI_H
#define OPTI_H

#include "util.h"
#include "quad.h"
#include "stable.h"
#include "list.h"

void optiLoop        (quad **code, symbol **gtos);
int optiDeadCode     (quad **code);
int optiVarDuplicate (quad **code);

#endif
