#ifndef STABLE_H
#define STABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "lstTab.h"
#define LEN 128

typedef enum stype {
    S_NONE, S_INT, S_BOOL, S_STRING, S_UNIT, S_LABEL, S_ARRAY
} stype;

//Bornes
typedef struct t_range {
    int min;
    int max;
    int ndim;
    struct t_range *next ;    
 } t_range;

//Structure d'un tableau
typedef struct s_array {
    stype type; //Type des valeurs du tableau
    int size;   //Taile totale du tableau ? Pas sure ?
    t_range *range; //Autant de struct t_range que de dimension ?

    union { // liste des valuers
        struct lstInt *intarr;
        struct lstBool *boolarr;
    };
} s_array;

typedef struct symbol {
    char  *id;
    bool  tmp; // is tmp var ? (true = cannot be modified by user)
    stype type;
    union {
        int  ival;
        char *sval;
        bool bval;
        s_array array;
    };
    struct symbol *next;
} symbol;

void   ferr         (char *s);
void   sFree        (symbol *);
symbol *sAlloc      ();
symbol *sAdd        (symbol **);
symbol *search      (symbol *, char *);

symbol *newVar      (symbol **, stype, char *, void *);
symbol *newTmpInt   (symbol **, int);
symbol *newTmpStr   (symbol **, char *);
symbol *newTmpBool  (symbol **, bool);
symbol *newTmpLabel (symbol **);
symbol *newVarInt   (symbol **, char *, int);
symbol *newVarStr   (symbol **, char *, char *);
symbol *newVarBool  (symbol **, char *, bool);
symbol *newVarUnit  (symbol **, char *);
symbol *newVarArray (symbol **, char *, s_array);

#endif
