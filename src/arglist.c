#include "../headers/arglist.h"

// sym: only for function args, put it to NULL for var declarations
arglist *arglistNew (char *id, symbol *sym) {
		arglist *al = malloc(sizeof(arglist));
		if (al == NULL)
				ferr(__LINE__ ,"arglist.c arglistNew malloc");

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
		printf("vars : [ ");

		while (l != NULL) {
				if (l->next != NULL)
						printf("%s, ", l->id);
				else
						printf("%s ] : ", l->id);
				l = l->next;
		}
}

// Utile pour convertir les arglist de function call en liste de symbol * pour passer en param de qGen
symbol *arglistToSymlist (arglist *al) {
		symbol *slist = NULL, *slast = NULL, *cur;

		while (al) {
				cur = sAdd(&slist);
				if (slast != NULL)
						slast->next = cur;

				cur->id = strdup(al->sym->id);
				if (cur->id == NULL)
						ferr(__LINE__ ,"arglist.c arglistToSymlist strdup");

				cur->tmp   = al->sym->tmp;
				cur->type  = al->sym->type;

				switch (cur->type) {
						case S_INT  : cur->ival = al->sym->ival; break;
						case S_BOOL : cur->bval = al->sym->bval; break;
						case S_ARRAY : cur->arr = al->sym->arr;  break;
						default: ferr(__LINE__ ,"arglist.c arglistToSymlist wrong type");
				}

				slast = cur;
				al    = al->next;
		}

		return slist;
}
