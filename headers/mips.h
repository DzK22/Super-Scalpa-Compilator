#ifndef MIPS_H
#define MIPS_H

#include "util.h"
#include "array.h"

void getMips (FILE *, symbol *, quad *);
void getData (FILE *f, symbol *s, int bytes);
void getText (FILE *f, quad *s);

void getDataArray (FILE *f, symbol *s);
void getMipsFunctionCompind   (FILE *f);
void getMipsInternalFunctions (FILE *f);

char * nextTmpLabel (void);
char * opstr        (qop);

// common
void arrComputeIndex (FILE *f, symbol *sarr, symbol *sargs);
void qAffect (FILE *f, symbol *res, symbol *argv1, symbol *argv2);
void qRead   (FILE *f, symbol *res, symbol *argv1);
void qWrite  (FILE *f, symbol *argv1);
void qArith  (FILE *f, qop op, symbol *res, symbol *argv1, symbol *argv2);
void qComp   (FILE *f, qop op, symbol *res, symbol *argv1, symbol *argv2);
void qNot    (FILE *f, symbol *res, symbol *argv1);

// functions
void fundec    (FILE *f, symbol *fun);
void funend    (FILE *f, symbol *fun);
void funcall   (FILE *f, symbol *fun, symbol *args, symbol *res);
void funreturn (FILE *f, symbol *fun, symbol *ret);

// function helpers
int  funSymTypeSize      (symbol *s);
int  funArgsStackSize    (symbol *fun);

void funStackLoadArgs    (FILE *f, symbol *fun);
void funStackPushArgs    (FILE *f, symbol *fun, symbol *args);
void funCopyArray        (FILE *f, symbol *destSym);
void funArgsDebugString  (symbol *fun, symbol *args, char *dstring, int maxlen);

int  curfunVarStackSize  (void);
void curfunStackPushVars (FILE *f);
void curfunStackLoadVars (FILE *f);
symbol * curfunNextUsefullLocalVar (symbol *tos);

#endif
