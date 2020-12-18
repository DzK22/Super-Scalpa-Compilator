#include "../headers/list.h"

list* newList (void) {
	return NULL;
}

list *addList (list *list, char *data) {
	list *item = malloc(sizeof(struct list));
	if (item == NULL) {
		fprintf(stderr, "malloc error\n");
		return NULL;
	}
	if ((item->tid = strdup(data)) == NULL) {
		fprintf(stderr, "strdup error\n");
		return NULL;
	}
	item->next = NULL;
	list->next = item;
	return list;
}
