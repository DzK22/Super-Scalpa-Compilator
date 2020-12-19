#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stable.h"

struct listIdents {
	char   *tid;
	struct listIdents *next;
	stype  type;
 };

struct listDecls {
	struct listIdents *list;
	struct listDecls  *next;
};

typedef struct listIdents listIdents;
typedef struct listDecls  listDecls;

listDecls *newList (listIdents*);
listDecls *addList (listDecls *, listIdents*);
void printID       (listIdents *);

#endif
