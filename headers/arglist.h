#ifndef ARGLIST_H
#define ARGLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stable.h"

typedef struct arglist {
	struct arglist *next;
	char *id;

	// juste pour les args de fonctions ci dessous, dont utiliser for now
	stype type; // type determine le value du union
	union {
		int  ival;
		bool bval;
		char *sval;
	};
} arglist;

arglist * arglistNew    (char *, stype, void *);
arglist * arglistConcat (arglist *, arglist *);
void arglistPrint       (arglist *);

#endif
