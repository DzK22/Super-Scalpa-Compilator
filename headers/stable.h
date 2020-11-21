#ifndef STABLE_H
#define STABLE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define LEN 64

typedef union {
    int v_int;
    void *v_empty;
    char *v_string;
} svalue;

typedef enum {INTEGER_, STRING_, NONE_} stype;

typedef struct symbol {
    bool cst;
    char *id;
    stype type;
    svalue value;
    struct symbol *next;
} symbol;

symbol *sAlloc (void);
symbol *sNewTemp (symbol **, stype);
void sFree (symbol *);
symbol *sNewCstInt (symbol **, int);
symbol *sNewStringInt (symbol **, char *);
symbol *sAdd (symbol **, char *, stype);

#endif
