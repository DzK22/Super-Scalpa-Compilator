#ifndef STABLE_H
#define STABLE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define LEN 64

typedef enum {INTEGER_, STRING_, NONE_} stype;

typedef struct symbol {
    bool cst;
    char *id;
    stype type;
    union {
        int value;
        char *string;
    };
    struct symbol *next;
} symbol;

symbol *newTemp (symbol **);
void   sFree (symbol *);
symbol *newCstInt (symbol **, int);
symbol *newCstString (symbol **, char *);
symbol *search (symbol *, char *);

#endif
