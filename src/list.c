#include "../headers/list.h"

listDecls *newList (listIdents *list) {
	listDecls *l = malloc(sizeof(listDecls));
	if (l == NULL)
		ferr("list.c:newList malloc");

	l->next = NULL;
	l->list = list;

	return l;
}

listDecls *addList (listDecls *listDecl, listIdents *listID) {
	listDecls *node = newList(listID);
	node->next      = listDecl->next;
	listDecl->next  = node;

	return node;
}

void printID (listIdents *list) {
	listIdents *cur = list;
	fprintf(stdout, "vars : [ ");

	while (cur != NULL) {
		if (cur->next != NULL)
			fprintf(stdout, "%s, ", cur->tid);
		else
			fprintf(stdout, "%s ]\n", cur->tid);
		cur = cur->next;
	}
}
