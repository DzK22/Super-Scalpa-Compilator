#include "../headers/lstTab.h"

lstInt *newLstInt (int ival) {
    lstInt *nl = malloc(sizeof(lstInt));
    if (nl == NULL) {
        fprintf(stderr, "malloc error\n");
        return NULL;
    }
    nl->ival = ival;
    nl->next = NULL;
    return nl;
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

lstBool *newLstBool (bool bval) {
  lstBool * nl = malloc(sizeof(lstBool));
  if ( nl == NULL ) {
    fprintf(stderr,"Malloc newLstBool error\n");
  }
  nl->bval = bval;
  nl->next = NULL;
  return nl;
}

void freeLstBool (lstBool * list) {
  lstBool *cur;
  while (list != NULL) {
    cur = list;
    list = list->next;
    free(cur);
  }
}

lstBool *addLstBool (lstBool * list, bool bval) {
  lstBool * item = newLstBool(bval);
  if ( list == NULL ) {
    return item;
  }
  lstBool *cur = list;
  while (cur->next != NULL)
      cur = cur->next;
  cur->next = item;
  return list;
}

bool getNthBoolVal (lstBool * list , int index){
  lstBool *cur = list;
  int i;
  for (i = 1; i < index && cur != NULL ; i++) {
    cur = cur->next;
  }
  if (cur == NULL || i < index - 1) {
      fprintf(stderr, "Out of bounds Bool array\n");
      exit(EXIT_FAILURE);
  }
  return cur->bval;
}
