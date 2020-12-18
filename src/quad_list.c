#include "../headers/quad_list.h"

quad_list *newQuadList (quad *quad) {
    quad_list *nqlist = malloc(sizeof(struct quad_list));
    nqlist->item = quad;
    nqlist->next = NULL;
    return nqlist;
}

quad_list *concatQuadList (quad_list *list1, quad_list *list2) {
    quad_list *cur = list1;
    if (cur == NULL)
        return list2;
    while (cur->next != NULL)
        cur = cur->next;
    cur->next = list2;
    return list1;
}

void completeQuadList (quad_list *list, symbol *goto_) {
    quad_list *cur = list;
    while (cur != NULL) {
        cur->item->res = goto_;
        cur = cur->next;
    }
    qlFree(list);
}

void qlFree(quad_list *list) {
    quad_list *cur = list, *last;
    while (cur != NULL) {
        last = cur;
        cur = cur->next;
        free(last);
    }
}
