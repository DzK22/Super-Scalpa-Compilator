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

dimProp *initDimProp   (int min, int max, dimProp *dp) {
	dimProp *nDp = malloc(sizeof(dimProp));
	if (nDp == NULL)
		ferr(__LINE__, "array.c malloc error\n");
	if (dp == NULL) {
		nDp->dim = 1;
		nDp->next = NULL;
	}
	else {
		nDp->dim = dp->dim + 1;
		nDp->next = dp;
	}
	nDp->min = min;
	nDp->max = max;
	return nDp;
}

s_array *initArray (dimProp *rangelist, stype type) {
	s_array *nArr = malloc(sizeof(struct s_array));
	if (nArr == NULL)
		ferr(__LINE__, "array.c malloc error");
	nArr->dims = rangelist;
	nArr->ndims = rangelist->dim;
	nArr->type = type;
	int size = 1;
	dimProp *cur = rangelist;
	while (cur != NULL) {
		size *= (cur->max - cur->min + 1);
		cur = cur->next;
	}
	nArr->size = size;
	return nArr;
}
