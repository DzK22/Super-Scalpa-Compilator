#include "../headers/list.h"

listDecls *newList (listIdents *list) {
	listDecls *l = malloc(sizeof(listDecls));
	if (l != NULL) {
		l->next = NULL;
		l->list = list;
	}
	return l;
}

listDecls *addList (listDecls *listDecl, listIdents *listID) {
	listDecls *node = newList(listID);
	if (node) {
		node->next = listDecl->next;
		listDecl->next = node;
	}
	return node;
}

void printID (listIdents *list) {
	listIdents *cur = list;
	while (cur != NULL) {
		fprintf(stdout, "%s\n", cur->tid);
		cur = cur->next;
	}
}
