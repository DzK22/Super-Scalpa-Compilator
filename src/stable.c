#include "../headers/stable.h"
#include "../headers/arglist.h"

symbol *sAlloc () {
    symbol *ns = malloc(sizeof(struct symbol));
    if (ns == NULL)
        ferr("stable.c sAlloc malloc");

    ns->id   = NULL;
    ns->tmp  = false;
    ns->type = S_NONE;
    ns->next = NULL;

    ns->ival = 0;
    ns->sval = NULL;
    ns->bval = false;

    return ns;
}

symbol *sAdd (symbol **stable) {
    if (*stable == NULL) {
        *stable = sAlloc();
        return *stable;

    } else {
        symbol *cur = *stable;
        while (cur->next != NULL)
            cur = cur->next;

        cur->next = sAlloc();
        return cur->next;
    }
}

void sFree (symbol *s) {
    symbol *cur = s;
    symbol *prev;

    while (cur != NULL) {
        prev = cur;
        cur = cur->next;
        free(prev->id);

        if (prev->type == S_STRING)
            free(prev->sval);

        free(prev);
    }
}

// ID == NULL => var temporaire
// data auto cast in the good type
// do not call this function directry, use helpers
symbol *newVar (symbol **stable, stype type, char *id, void *data) {
    static int nsym = 0;
    static int nlabels = 0;
    bool isTmp;
    if (id != NULL)
        isTmp = false;
    else
        isTmp = true;

    char tid[LEN];
    int res;

    if (isTmp) {
        if (type == S_LABEL)
            res = snprintf(tid, LEN, "label_%d", nlabels ++);
        else
            res = snprintf(tid, LEN, "temp_%d", nsym ++);

    } else {
        if (search(*stable, id) != NULL)
            ferr("stable.c newVar var ID already exists");

        if (type == S_PROG)
            res = sprintf(tid, "prog_%s", id);
        else if (type == S_FUNCTION)
            res = sprintf(tid, "fun_%s", id);
        else
            res = sprintf(tid, "var_%s", id);
    }

    if (res < 0 || res >= LEN)
        ferr("stable.c newVar tid snprintf");

    symbol *nt = sAdd(stable);
    if ((nt->id = strdup(tid)) == NULL)
        ferr("stable.c newVar strdup data ID");

    nt->type = type;
    nt->tmp  = isTmp;

    switch (type) {
        case S_INT:
            nt->ival = *((int *) data);
            break;
        case S_BOOL:
            nt->bval = *((bool *) data);
            break;
        case S_STRING:
            if ((nt->sval = strdup((char *) data)) == NULL)
                ferr("stable.c newVar strdup data");
            break;
        case S_LABEL:
            if ((nt->sval = strdup(tid)) == NULL)
                ferr("stable.c newVar strdup data");
            break;
        case S_FUNCTION:
            nt->fdata = data;
            break;
        case S_PROG:
            break;
        default:
            ferr("stable.c newVar unknow type");
    }

    return nt;
}

// helpers functions which call newVar

symbol *newTmpInt (symbol **stable, int val) {
    return newVar(stable, S_INT, NULL, &val);
}

symbol *newTmpStr (symbol **stable, char *str) {
    return newVar(stable, S_STRING, NULL, str);
}

symbol *newTmpBool (symbol **stable, bool bol) {
    return newVar(stable, S_BOOL, NULL, &bol);
}

symbol *newTmpLabel (symbol **stable) {
    return newVar(stable, S_LABEL, NULL, NULL);
}

symbol *newVarInt (symbol **stable, char *id, int val) {
    return newVar(stable, S_INT, id, &val);
}

symbol *newVarStr (symbol **stable, char *id, char *str) {
    return newVar(stable, S_STRING, id, str);
}

symbol *newVarBool (symbol **stable, char *id, bool bol) {
    return newVar(stable, S_BOOL, id, &bol);
}

symbol *newVarUnit (symbol **stable, char *id) {
    return newVar(stable, S_UNIT, id, NULL);
}

symbol *newVarFun (symbol **stable, char *id, arglist *al, stype rtype) {
    fundata *fdata = malloc(sizeof(fundata));
    fdata->al      = al;
    fdata->rtype   = rtype;

    return newVar(stable, S_FUNCTION, id, fdata);
}

symbol *newProg (symbol **stable, char *id) {
    return newVar(stable, S_PROG, id, NULL);
}

symbol *search (symbol *stable, char *id) {
    char var[LEN], fun[LEN];
    int res;
    res = snprintf(var, LEN, "var_%s", id);
    if (res < 0 || res >= LEN)
        ferr("stable.c search snprintf");
    res = snprintf(fun, LEN, "fun_%s", id);
    if (res < 0 || res >= LEN)
        ferr("stable.c search snprintf");

    while (stable != NULL) {
        if (((strcmp(stable->id, var)) == 0) || (strcmp(stable->id, fun) == 0))
            return stable;
        stable = stable->next;
    }

    return NULL;
}

void stablePrint  (symbol *stable) {
    while (stable != NULL) {
        fprintf(stdout, "%s: ", stable->id);
        switch (stable->type) {
            case S_INT:
                fprintf(stdout, "INTEGER\n");
                break;
            case S_BOOL:
                fprintf(stdout, "BOOLEAN\n");
                break;
            case S_ARRAY:
                fprintf(stdout, "ARRAY\n"); // ajouter type de valeurs quand tableaux seront faits
                break;
            case S_STRING:
                fprintf(stdout, "STRING\n");
                break;
            case S_PROG:
                fprintf(stdout, "PROGRAM NAME\n");
                break;
            case S_FUNCTION:
                fprintf(stdout, "Function : returns => ");
                switch (((fundata *)stable->fdata)->rtype) {
                    case S_INT:
                        fprintf(stdout, "INTEGER\n");
                        break;
                    case S_BOOL:
                        fprintf(stdout, "BOOLEAN\n");
                        break;
                    case S_UNIT:
                        fprintf(stdout, "UNIT\n");
                        break;
                }
                break;
        }
        //fprintf(stdout, "\n");
        stable = stable->next;
    }
}
