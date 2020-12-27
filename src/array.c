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
