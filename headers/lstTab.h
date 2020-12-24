#ifndef LST_TAB_H
#define LST_TAB_H

#include "stable.h"

typedef struct lstInt {
    int ival;
    struct lstInt *next;
} lstInt;

lstInt *newLstInt (int);
void freeLstInt (lstInt *);
lstInt *addLstInt (lstInt *, int);
int getNthIntVal (lstInt *, int);


#endif
