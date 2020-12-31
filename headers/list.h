#ifndef LIST_H
#define LIST_H

#include "util.h"
#include "stable.h"
#include "types.h"

typedef struct list {
	struct list *next;
	char   *id;  // only for var decl
	struct symbol *sym; // only for fun args
} list;

typedef struct fundata {
	list *al;
	stype rtype;
	struct symbol *tos;
} fundata;

typedef struct exprlist {
	struct quad    *quad;
	struct list *al;
} exprlist;

list * listNew    (char *, struct symbol *);
list * listConcat (list *, list *);
void listPrint    (list *);
struct symbol  * listToSymlist (list *);
list * symListToList (struct symbol *sl);

#endif
