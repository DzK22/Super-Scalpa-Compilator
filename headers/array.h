#ifndef ARRAY_H
#define ARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quad.h"
#include "arglist.h"
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
    dimProp *dims;    // pointeur sur la première dimension
    struct arglist *args; // only for array index calculation tmp
    stype   type;
} s_array;

typedef struct rlist {
	struct rlist *next;
	union {
		struct arglist *al;
		dimProp *dp;
	};
} rlist;

rlist *rlistNew        (struct arglist *al, dimProp *dp);
s_array *initArray     (dimProp *rangelist, stype type);
dimProp *initDimProp   (int min, int max, dimProp *dp);

#endif
