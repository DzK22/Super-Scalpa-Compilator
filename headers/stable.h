#ifndef STABLE_H
#define STABLE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define LEN 128

typedef enum stype {
    S_NONE, S_INT, S_BOOL, S_STRING, S_UNIT
} stype;

typedef struct symbol {
    char  *id;
    bool  tmp; // is tmp var ? (true = cannot be modified by user)
    stype type;
    union {
        int  ival;
        char *sval;
        bool bval;
    };
    struct symbol *next;
} symbol;

void ferr          (char *s);
void   sFree       (symbol *);
symbol *sAlloc     ();
symbol *sAdd       (symbol **);
symbol *search     (symbol *, char *);

symbol *newVar     (symbol **, stype, char *, void *);
symbol *newTmpInt  (symbol **, int);
symbol *newTmpStr  (symbol **, char *);
symbol *newTmpBool (symbol **, bool);

symbol *newVarInt  (symbol **, char *, int);
symbol *newVarStr  (symbol **, char *, char *);
symbol *newVarBool (symbol **, char *, bool);
symbol *newVarUnit (symbol **, char *);

#endif
