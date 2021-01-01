#include "../headers/array.h"

// obtain a reverse list of list* or dimProp*
rlist *rlistNew (list *al, dimProp *dp) {
	if (al && dp)
		ferr("rlistNew - only one of al or dp should be set");

	rlist *rl = NULL, *last = NULL;

	while (al != NULL || dp != NULL) {
		rl = malloc(sizeof(rlist));
		if (rl == NULL)
			ferr("rlistNew - malloc error");

		rl->next = last;
		last     = rl;

		rl->al = NULL;
		rl->dp = NULL;

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

void rlistFree (rlist *rl){
	rlist *prec;

	while (rl != NULL) {
		prec = rl;
		rl = rl->next;

		if (prec->al != NULL)
			listFree(prec->al);
		if (prec->dp != NULL)
			freeDimProp(prec->dp);
			
		free(prec);
	}
}

dimProp *initDimProp (int min, int max, dimProp *dp) {
	dimProp *nDp = malloc(sizeof(dimProp));
	if (nDp == NULL)
		ferr("malloc error\n");

	if (dp == NULL) {
		nDp->dim = 1;
		nDp->next = NULL;
	} else {
		nDp->dim = dp->dim + 1;
		nDp->next = dp;
	}

	nDp->min = min;
	nDp->max = max;
	return nDp;
}

void freeDimProp (dimProp *dp) {
	dimProp *prec;

	while (dp != NULL) {
		prec = dp;
		dp = dp->next;
		free(prec);
	}
}

s_array *initArray (dimProp *rangelist, stype type) {
	s_array *nArr = malloc(sizeof(struct s_array));
	if (nArr == NULL)
		ferr("malloc error");

	nArr->dims   = rangelist;
	nArr->ndims  = rangelist->dim;
	nArr->type   = type;
	dimProp *cur = rangelist;
	int size     = 1;

	while (cur != NULL) {
		size *= (cur->max - cur->min + 1);
		cur = cur->next;
	}

	nArr->size = size;
	return nArr;
}

bool testArrayIndices (dimProp *dp, struct list *al) {
	int len = 1;
	struct list *cur = al;

	while (cur != NULL) {
		len ++;
		cur = cur->next;
	}

	return (len - 1) == dp->dim;
}
