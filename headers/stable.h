#ifndef STABLE_H
#define STABLE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define LEN 128

typedef enum {INTEGER_, STRING_, NONE_} stype;

typedef struct symbol {
    char *id;
    bool cst;
    bool init;
    stype type;
    union {
        int value;
        char *string;
    };
    struct symbol *next;
} symbol;

symbol *sAlloc ();
symbol *sAdd (symbol **, char *);
symbol *newTemp (symbol **);
void   sFree (symbol *);
symbol *newCstInt (symbol **, int);
symbol *newCstString (symbol **, char *);
symbol *search (symbol *, char *);
void ferr (char *s);

#endif
