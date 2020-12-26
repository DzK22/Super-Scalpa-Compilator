#include "../headers/array.h"

// obtain a reverse list of arglist* or dimProp*
rlist *rlistNew (arglist *al, dimProp *dp) {
	if (al && dp)
		ferr("array.c rlistNew - only one of al or dp should be set");

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
