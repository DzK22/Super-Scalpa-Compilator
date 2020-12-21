#ifndef MIPS_H
#define MIPS_H

#define MIPS_BUFFER_SPACE 32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "quad.h"
#include "stable.h"

void getMips (FILE *, symbol *, quad *);
void getData (FILE *f, symbol *s);
void getText (FILE *f, quad *s);
void ferr    (char *s);
char * nextTmpLabel ();
char * opstr (qop);

#endif
