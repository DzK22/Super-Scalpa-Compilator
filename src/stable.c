#include "../headers/stable.h"

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
    bool isTmp      = id ? false : true;
    char *finalID   = id;
    char tid[LEN];

    if (isTmp) { // is tmp var
        int res = snprintf(tid, LEN, "temp_%d", nsym ++);
        if (res < 0 || res >= LEN)
            ferr("stable.c newVar snprintf");
        finalID = tid;
    }

    symbol *nt = sAdd(stable);
    if ((nt->id = strdup(finalID)) == NULL)
        ferr("stable.c newVar strdup data ID");

    nt->type = type;
    nt->tmp  = isTmp;

    if (type == S_INT)
        nt->ival = *((int *) data);
    else if (type == S_BOOL)
        nt->bval = *((bool *) data);
    else if (type == S_STRING) {
        if ((nt->sval = strdup((char *) data)) == NULL)
            ferr("stable.c newVar strdup data");
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

symbol *search (symbol *s, char *id) {
    while (s != NULL) {
        if (strcmp(s->id, id) == 0)
            return s;
        s = s->next;
    }

    return NULL;
}
