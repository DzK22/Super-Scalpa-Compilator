#ifndef LST_TAB_H
#define LST_TAB_H

#include "stable.h"

typedef struct lstBool {
    bool bval;
    struct lstBool *next;
} lstBool;

typedef struct lstInt {
    int ival;
    struct lstInt *next;
} lstInt;

lstInt *newLstInt (int);
void freeLstInt (lstInt *);
lstInt *addLstInt (lstInt *, int);
int getNthIntVal (lstInt *, int);

lstBool *newLstBool (int );
void freeLstBool (lstBool *);
lstBool *addLstBool (lstBool *, bool);
bool getNthBoolVal (lstBool *, int);


#endif
