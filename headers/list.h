#ifndef _LIST_H
#define _LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct list {
	char *tid;
	struct list *next;
} list;

list *newList (void);
list *addList (list *, char *);

#endif
