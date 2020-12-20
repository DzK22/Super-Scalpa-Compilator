#include "../headers/arglist.h"

// mettre type = S_NONE si pas besoin d'avoir de type (ex: liste declaration de variables, juste les ID qui sont usefull mdr
arglist *arglistNew (char *id, stype type, void *val) {
	arglist *al = malloc(sizeof(arglist));
	if (al == NULL)
		ferr("arglist.c arglistNew malloc");

	al->next = NULL;
	al->id   = id;
	al->type = type;

	if (val != NULL) {
		if (type == S_INT)
			al->ival  = *((int *) val);
		else if (type == S_BOOL)
			al->bval  = *((bool *) val);
		else if (type == S_STRING)
			al->sval  = (char *) val;
	}

	return al;
}

arglist * arglistConcat (arglist *l1, arglist *l2) {
	if (l1 == NULL)
		return l2;

	arglist *l = l1, *last;
	while (l != NULL) {
		last = l;
		l = l->next;
	}

	last->next = l2;
	return l1;
}

void arglistPrint (arglist *la) {
	arglist *l = la;
	fprintf(stdout, "vars : [ ");

	while (l != NULL) {
		if (l->next != NULL)
			fprintf(stdout, "%s, ", l->id);
		else
			fprintf(stdout, "%s ]\n", l->id);
		l = l->next;
	}
}
