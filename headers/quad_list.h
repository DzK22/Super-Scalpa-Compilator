#ifndef QUAD_LIST_H
#define QUAD_LIST_H
#include "quad.h"
#include "stable.h"

typedef struct quad_list {
    quad *item;
    struct quad_list *next;
} quad_list;

quad_list *newQuadList (quad *);
quad_list *concatQuadList (quad_list *, quad_list *);
void completeQuadList (quad_list *, symbol *);
void qlFree(quad_list *);

#endif
