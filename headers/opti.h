#ifndef OPTI_H
#define OPTI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "quad.h"
#include "stable.h"
#include "arglist.h"

void optiLoop        (quad **code, symbol **gtos);
int optiDeadCode     (quad **code);
int optiVarDuplicate (quad **code);

#endif

