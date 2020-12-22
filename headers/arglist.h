#ifndef ARGLIST_H
#define ARGLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stable.h"
#include "quad.h"

typedef struct arglist {
	struct arglist *next;
	char   *id;
	symbol *sym; // arg symbol (only for function args)
} arglist;

typedef struct fundata {
    arglist *al;
    stype rtype;
} fundata;

typedef struct exprlist {
	struct quad    *quad;
	struct arglist *al;
} exprlist;

arglist * arglistNew     (char *, symbol *);
arglist * arglistConcat  (arglist *, arglist *);
void arglistPrint        (arglist *);
symbol *arglistToSymlist (arglist *);

// dans stable.h normalement, mais pb inclusion en boucle ou jsp quoi mdr interblocage du pauvre de declaration mdr
symbol *newVarFun        (symbol **, char *, arglist *, stype);

#endif
