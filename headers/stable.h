#ifndef STABLE_H
#define STABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#define LEN 128
#define CAPACITY 500000

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
    int     ndims;
    int     size;
    dimProp *dims;    // pointeur sur la première dimension
    stype   type;
    int     index;
} s_array;

//Hash Item KEY = symbol->id
typedef struct symbol {
    char   *id;
    bool   tmp; // is tmp var ? (true = cannot be modified by user)
    stype  type;
    bool   ref;
    struct symbol *next;

    union {
        int32_t ival;   // integer
        int8_t  bval;   // boolean
        char    *sval;  // string

        void    *fdata; // function data ( = struct fundata)
        s_array *arr;   // array
    };
} symbol;

typedef struct hash_item {
    struct hash_item *next; //Pour gérer les collisions
    void *data;             //Pour pouvoir refactorer les symboles plus tard sans avoir de pb de compil haha
    char *key;              //Clé qui sera l'identificateur du symbol
} hash_item;

typedef struct h_table {
    hash_item **stable;
    int size;
    int count;
} hashtable;

void   ferr         (int, char *s);
void   sFree        (symbol *);
symbol *sAlloc      (void);
symbol *sAdd        (symbol **);
symbol *searchTable (symbol *, char *, symbol *);
symbol *search      (symbol *, symbol *, char *);

symbol *newVar      (symbol **, stype, char *, void *, symbol *, bool);
symbol *newTmpInt   (symbol **, int);
symbol *newTmpStr   (symbol **, char *);
symbol *newTmpBool  (symbol **, bool);
symbol *newTmpLabel (symbol **);
symbol *newVarInt   (symbol **, char *, int, symbol *, bool);
symbol *newVarStr   (symbol **, char *, char *, symbol *);
symbol *newVarBool  (symbol **, char *, bool, symbol *, bool);
symbol *newVarFun   (symbol **, char *);
symbol *newProg     (symbol **, char *);
symbol *newVarArray (symbol **, char *, s_array *, symbol *, bool);

void   stablePrint  (symbol *);

/************************/
/*                      */
/* HASHTABLE FUNCTIONS  */
/*                      */
/************************/
unsigned long getHash (char *key);
hashtable *initHashTable (int size);
//void freeHashTable (hashtable *htable);
void *insertHashTable (hashtable *htable, char *key, void *data);

#endif
