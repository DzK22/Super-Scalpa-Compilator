#include "../headers/list.h"

// sym: only for function args, put it to NULL for var declarations
list *listNew (char *id, symbol *sym) {
		list *al = malloc(sizeof(list));
		if (al == NULL)
				ferr("listNew malloc");

		al->next = NULL;
		al->id   = id;
		al->sym  = sym;

		return al;
}

void listFree (list *l) {
	list *prec;

	while (l != NULL) {
		prec = l;
		l = l->next;

		if (prec->id != NULL)
			free(prec->id);
		free(prec);
	}
}

list * listConcat (list *l1, list *l2) {
		if (l1 == NULL)
				return l2;

		list *l = l1, *last;
		while (l != NULL) {
				last = l;
				l = l->next;
		}

		last->next = l2;
		return l1;
}

void listPrint (list *la) {
		list *l = la;
		printf("vars : [ ");

		while (l != NULL) {
				if (l->next != NULL)
					printf("%s, ", l->id);
				else
					printf("%s ] : ", l->id);
				l = l->next;
		}
}

// Utile pour convertir les list de function call en liste de symbol * pour passer en param de qGen
symbol * listToSymlist (list *al) {
		symbol *slist = NULL, *slast = NULL, *cur;

		while (al) {
				cur = sAdd(&slist);
				if (slast != NULL)
						slast->next = cur;

				cur->id   = al->sym->id;
				cur->tmp  = al->sym->tmp;
				cur->type = al->sym->type;
				cur->ref  = al->sym->ref;

				switch (cur->type) {
						case S_INT    : cur->ival = al->sym->ival ; break;
						case S_BOOL   : cur->bval = al->sym->bval ; break;
						case S_ARRAY  : cur->arr  = al->sym->arr  ; break;
						case S_STRING : cur->sval = al->sym->sval ; break;
						default: ferr("listToSymlist wrong type");
				}

				slast = cur;
				al    = al->next;
		}

		return slist;
}

void symListFree (symbol *sl) {
	symbol *last;

	while (sl != NULL) {
		last = sl;
		sl = sl->next;
		free(last);
	}
}

list *symListToList (symbol *sl) {
	list *res = NULL, *l;
	char *id;

	while (sl != NULL) {
		if ((id = strdup(sl->id)) == NULL)
			ferr("symListToList strdup id");

		l   = listNew(id, sl);
		res = listConcat(res, l);
		sl  = sl->next;
	}

	return res;
}
