#ifndef STABLE_H
#define STABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define LEN 128

typedef enum stype {
    S_NONE, S_INT, S_BOOL, S_STRING, S_UNIT, S_LABEL, S_ARRAY, S_FUNCTION, S_PROG
} stype;

typedef struct arr_range {
    int min;
    int max;
    int dim;
    struct arr_range *next;
} dimProp;

typedef struct s_array {
    int ndims;
    int size;
    dimProp *dims;           //pointeur sur la première dimension
    int *values;  //Liste des valeurs
    stype type;
    int index;
} s_array;

typedef struct symbol {
    char   *id;
    bool   tmp; // is tmp var ? (true = cannot be modified by user)
    stype  type;
    struct symbol *next;

    union {
        int  ival;
        char *sval;
        bool bval;
        void *fdata; // function data ( = fundata)
        s_array *arr;
        // probleme = on peut pas declarer arglist fdata sinon ca cause interblocage du pauvre de déclarations entre arglist et symbol des fichiers stable.h et arglist.h
    };
} symbol;

void   ferr         (char *s);
void   sFree        (symbol *);
symbol *sAlloc      ();
symbol *sAdd        (symbol **);
symbol *searchTable (symbol *, char *, symbol *);
symbol *search      (symbol *, symbol *, char *);

symbol *newVar      (symbol **, stype, char *, void *, symbol *);
symbol *newTmpInt   (symbol **, int);
symbol *newTmpStr   (symbol **, char *);
symbol *newTmpBool  (symbol **, bool);
symbol *newTmpLabel (symbol **);
symbol *newVarInt   (symbol **, char *, int, symbol *);
symbol *newVarStr   (symbol **, char *, char *, symbol *);
symbol *newVarBool  (symbol **, char *, bool, symbol *);
symbol *newVarFun   (symbol **, char *);
symbol *newProg     (symbol **, char *);
symbol *newVarArray (symbol **, char *, s_array *);

void   stablePrint  (symbol *);

#endif
