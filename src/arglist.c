#include "../headers/arglist.h"

// sym: only for function args, put it to NULL for var declarations
arglist *arglistNew (char *id, symbol *sym) {
	arglist *al = malloc(sizeof(arglist));
	if (al == NULL)
		ferr("arglist.c arglistNew malloc");

	al->next = NULL;
	al->id   = id;
	al->sym  = sym;

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
			fprintf(stdout, "%s ] : ", l->id);
		l = l->next;
	}
}

// Utile pour convertir les arglist de function call en liste de symbol * pour passer en param de qGen
symbol *arglistToSymlist (arglist *al) {
	symbol *slist = NULL, *slast = NULL;

	printf("\n =====> ");
	while (al) {
		printf(" %s, ", al->sym->id);
		al = al->next;
	}
	printf("\n\n");

	while (al) {
		slist = sAdd(&slist);
		if (slast != NULL)
			slast->next = slist;

		slist->id = strdup(al->sym->id);
		if (slist->id == NULL)
			ferr("arglist.c arglistToSymlist strdup");

		slist->tmp   = al->sym->tmp;
		slist->type  = al->sym->type;

		switch (slist->type) {
			case S_INT  : slist->ival = al->sym->ival; break;
			case S_BOOL : slist->bval = al->sym->bval; break;
			default: ferr("arglist.c arglistToSymlist wrong slist type");
		}

		slast = slist;
		al    = al->next;
	}

	return slist;
}
