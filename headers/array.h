#ifndef ARRAY_H
#define ARRAY_H

#include "util.h"
#include "quad.h"
#include "list.h"
#include "stable.h"

typedef struct arr_range {
	int min;
	int max;
	int dim;
	struct arr_range *next;
} dimProp;

typedef struct s_array {
	int     ndims;
	int     size;
	dimProp *dims;         // pointeur sur la première dimension
	stype   type;
} s_array;

typedef struct rlist {
	struct rlist *next;
	struct list *al;
	dimProp *dp;
} rlist;

rlist *rlistNew       (struct list *al, dimProp *dp);
void rlistFree        (struct rlist *rl);

s_array *initArray    (dimProp *rangelist, stype type);
dimProp *initDimProp  (int min, int max, dimProp *dp);
void freeDimProp      (dimProp *dp);
bool testArrayIndices (dimProp *dp, struct list *al);

#endif
