#ifndef ARGLIST_H
#define ARGLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stable.h"
#include "types.h"

typedef struct arglist {
	struct arglist *next;
	char   *id;  // only for var decl
	struct symbol *sym; // only for fun args
} arglist;

typedef struct fundata {
    arglist *al;
    stype rtype;
	struct symbol *tos;
} fundata;

typedef struct exprlist {
	struct quad    *quad;
	struct arglist *al;
} exprlist;

arglist * arglistNew       (char *, struct symbol *);
arglist * arglistConcat    (arglist *, arglist *);
void arglistPrint          (arglist *);
struct symbol  * arglistToSymlist (arglist *);

#endif
