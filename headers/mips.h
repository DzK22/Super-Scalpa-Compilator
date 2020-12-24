#ifndef MIPS_H
#define MIPS_H

#define MIPS_BUFFER_SPACE 32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "quad.h"
#include "stable.h"
#include "arglist.h"

void getMips (FILE *, symbol *, quad *);
void getData (FILE *f, symbol *s);
void getText (FILE *f, quad *s);
void ferr    (char *s);
char * nextTmpLabel ();
char * opstr (qop);

// functions
void fundec    (FILE *f, symbol *fun);
void funend    (FILE *f, symbol *fun);
void funcall   (FILE *f, symbol *fun, symbol *args, symbol *res);
void funreturn (FILE *f, symbol *fun, symbol *ret);

// function helpers
int funArgsSize         (symbol *fun);
void funStackLoadArgs   (FILE *f, symbol *fun, int offset);
void funStackPushArgs   (FILE *f, symbol *args);
void funArgsDebugString (symbol *fun, char *dstring, int maxlen);

#endif
