#ifndef OPTI_H
#define OPTI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "quad.h"
#include "stable.h"
#include "arglist.h"

void optiLoop        (quad *q, symbol *gtos);
int optiDeadCode     (quad *q);
int optiVarDuplicate (quad *q);

#endif

