#ifndef ARRAY_H
#define ARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stable.h"
#include "quad.h"
#include "arglist.h"

typedef struct rlist {
	struct rlist *next;
	union {
		arglist *al;
		dimProp *dp;
	};
} rlist;

rlist *rlistNew        (arglist *al, dimProp *dp);

#endif
