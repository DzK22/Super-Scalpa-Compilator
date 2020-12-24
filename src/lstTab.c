#include "../headers/lstTab.h"

lstInt *newLstInt (int ival) {
  printf(">>>>>>>>>>>> taille initiale de tableau debut  %d \n",ival);

  lstInt  *tete = NULL , *q = NULL ;
  for (int i = 0; i < ival; i++) {

    lstInt *p = malloc(sizeof(lstInt));
    if (p == NULL) {
        fprintf(stderr, "malloc error lst int \n");
        return NULL;
    }
    p->ival = 0 ;
    p->next = NULL;

    if(tete == NULL)
      tete = p ;
    else {
      q = tete ;
       while (q->next != NULL) {
         q = q->next;
      }
      q->next = p;

    }
}

    return tete  ;

}

void freeLstInt (lstInt *list) {
    lstInt *cur;
    while (list != NULL) {
        cur = list;
        list = list->next;
        free(cur);
    }
}

lstInt *addLstInt (lstInt *list, int ival) {
    lstInt *item = malloc(sizeof(lstInt));
    if (item == NULL) {
        fprintf(stderr, "malloc error\n");
        return NULL;
    }
    item->ival = ival;
    item->next = NULL;
    if (list == NULL)
        return item;
    lstInt *cur = list;
    while (cur->next != NULL)
        cur = cur->next;
    cur->next = item;
    return list;
}

int getNthIntVal (lstInt *list, int nth) {
    lstInt *cur = list;
    int i = 0;
    for (i = 1; i < nth && cur != NULL; i++)
        cur = cur->next;
    if (cur == NULL || i < nth - 1) {
        fprintf(stderr, "out of bounds array \n");
        exit(EXIT_FAILURE);
    }
    return cur->ival;
}
