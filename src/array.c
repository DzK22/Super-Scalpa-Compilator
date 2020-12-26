#include "../headers/array.h"

// obtain a reverse list of arglist* or dimProp*
rlist *rlistNew (arglist *al, dimProp *dp) {
	if (al && dp)
		ferr(__LINE__ ,"array.c rlistNew - only one of al or dp should be set");

	rlist *rl = NULL, *last = NULL;

	while (al != NULL || dp != NULL) {
		rl       = malloc(sizeof(rlist));
		rl->next = last;
		last     = rl;

		if (al)
			rl->al = al;
		else
			rl->dp = dp;

		if (al)
			al = al->next;
		else
			dp = dp->next;
	}

	return rl;
}

void rlistPrint (rlist *rl) {
	int i = 0;
	while (rl) {
		printf(" [%d] => ", i ++);
		if (rl->al)
			printf("al sym ival = %d", rl->al->sym->ival);
		else
			printf("dp (dim, min, max) = (%d, %d, %d)", rl->dp->dim, rl->dp->min, rl->dp->max);

		printf("\n");
		rl = rl->next;
	}
}

// calcul de la valeur de l'indice du tableau
void arrayComputeIndex (arglist *indices, symbol *sarr) {
	dimProp *dimensions = sarr->arr->dims;

	rlist *rlal = rlistNew(indices, NULL);
	rlist *rldp = rlistNew(NULL, dimensions);

	int ind, min, max;
	int cnt = 1, factor = 1, index = 1;

	while (rlal && rldp) {
		ind = rlal->al->sym->ival;
		min = rldp->dp->min;
		max = rldp->dp->max;

		if (ind < min || ind > max)
			ferr(__LINE__ ,"array.c arrayComputeIndex - ind out of bound");

		index  += (ind - min) * factor;
		factor *= max - min + 1;
		cnt ++;

		rlal = rlal->next;
		rldp = rldp->next;
	}

	sarr->arr->index = index;
	if (index > sarr->arr->size)
		ferr(__LINE__ ,"array.c arrayComputeIndex - ind > arr size");
}
