#ifndef STABLE_H
#define STABLE_H

#include "util.h"
#include "array.h"
#include "list.h"

#define LEN       512

typedef struct symbol {
    char   *id;
    bool   tmp; // is tmp var ? (true = cannot be modified by user)
    stype  type;
    bool   ref;
    bool   is_cst;
    struct symbol *next;
    union {
        int32_t ival;   // integer
        int8_t  bval;   // boolean
        char    *sval;  // string

        struct fundata *fdata; // function data ( = struct fundata)
        struct s_array *arr;   // array
    };
} symbol;

void   sFree        (symbol *);
symbol *sAlloc      (void);
symbol *sAdd        (symbol **);
void    sDel        (symbol *, symbol *);
symbol *searchTable (symbol *, char *, symbol *);
symbol *search      (symbol *, symbol *, char *);

symbol *newVar      (symbol **, stype, char *, void *, symbol *, bool);
symbol *newTmpInt   (symbol **, int32_t);
symbol *newTmpStr   (symbol **, char *);
symbol *newTmpBool  (symbol **, int8_t);
symbol *newTmpLabel (symbol **);
symbol *newVarInt   (symbol **, char *, int32_t, symbol *, bool);
symbol *newVarStr   (symbol **, char *, char *, symbol *);
symbol *newVarBool  (symbol **, char *, int8_t, symbol *, bool);
symbol *newVarFun   (symbol **, char *);
symbol *newProg     (symbol **, char *);
symbol *newVarArray (symbol **, char *, struct s_array *, symbol *, bool);

void   stablePrint  (symbol *);
void   stablePrintAll (symbol *);

#endif
