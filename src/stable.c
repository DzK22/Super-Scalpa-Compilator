#include "../headers/stable.h"
#include "../headers/arglist.h"

symbol *sAlloc (void) {
    symbol *ns = malloc(sizeof(struct symbol));
    if (ns == NULL)
        ferr(__LINE__ ,"stable.c sAlloc malloc");

    ns->id   = NULL;
    ns->tmp  = false;
    ns->type = S_NONE;
    ns->next = NULL;

    ns->ival = 0;
    ns->sval = NULL;
    ns->bval = false;

    return ns;
}

symbol *sAdd (symbol **tos) {
    if (*tos == NULL) {
        *tos = sAlloc();
        return *tos;

    } else {
        symbol *cur = *tos;
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

/**
 * ID == NULL => var temporaire
 * data auto cast in the good type
 * do not call this function directry, use helpers
 * @param curfun Current function symbol or NULL if not in function
 * @param ref True is var contains a reference to a var of this type
 */
symbol *newVar (symbol **tos, stype type, char *id, void *data, symbol *curfun, bool ref) {
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
        if (searchTable(*tos, id, curfun) != NULL)
            ferr(__LINE__ ,"stable.c newVar var ID already exists");

        if (type == S_PROG)
            res = sprintf(tid, "prog_%s", id);
        else if (type == S_FUNCTION)
            res = sprintf(tid, "fun_%s", id);
        else { // int, bool, string, array
            if (curfun == NULL)
                res = sprintf(tid, "var_%s", id);
            else
                res = sprintf(tid, "funvar_%s_%s", curfun->id, id);
        }
    }

    if (res < 0 || res >= LEN)
        ferr(__LINE__ ,"stable.c newVar tid snprintf");

    symbol *nt = sAdd(tos);
    if ((nt->id = strdup(tid)) == NULL)
        ferr(__LINE__ ,"stable.c newVar strdup data ID");

    nt->type = type;
    nt->tmp  = isTmp;
    nt->ref  = ref;

    switch (type) {
        case S_INT:
            if (!ref)
                nt->ival = *((int *) data);
            break;
        case S_BOOL:
            if (!ref)
                nt->bval = *((bool *) data);
            break;
        case S_STRING:
            if ((nt->sval = strdup((char *) data)) == NULL)
                ferr(__LINE__ ,"stable.c newVar strdup data");
            break;
        case S_LABEL:
            if ((nt->sval = strdup(tid)) == NULL)
                ferr(__LINE__ ,"stable.c newVar strdup data");
            break;
        case S_FUNCTION:
            nt->fdata = data;
            break;
        case S_ARRAY:
            nt->arr = (s_array *) data;
            break;
        case S_PROG:
            break;
        default:
            ferr(__LINE__ ,"stable.c newVar unknow type");
    }

     return nt;
}

// Helpers functions which call newVar

symbol *newTmpInt (symbol **tos, int val) {
    return newVar(tos, S_INT, NULL, &val, NULL, false);
}

symbol *newTmpStr (symbol **tos, char *str) {
    return newVar(tos, S_STRING, NULL, str, NULL, false);
}

symbol *newTmpBool (symbol **tos, bool bol) {
    return newVar(tos, S_BOOL, NULL, &bol, NULL, false);
}

symbol *newTmpLabel (symbol **tos) {
    return newVar(tos, S_LABEL, NULL, NULL, NULL, false);
}

symbol *newVarInt (symbol **tos, char *id, int val, symbol *curfun, bool ref) {
    return newVar(tos, S_INT, id, &val, curfun, ref);
}

symbol *newVarStr (symbol **tos, char *id, char *str, symbol *curfun) {
    return newVar(tos, S_STRING, id, str, curfun, false);
}

symbol *newVarBool (symbol **tos, char *id, bool bol, symbol *curfun, bool ref) {
    return newVar(tos, S_BOOL, id, &bol, curfun, ref);
}

symbol *newVarFun (symbol **tos, char *id) {
    fundata *fdata = malloc(sizeof(fundata));
    fdata->al      = NULL;
    fdata->rtype   = S_NONE;
    fdata->tos     = NULL;
    // do not forget to set fdata->al and fdata->rtype after init !

    return newVar(tos, S_FUNCTION, id, fdata, NULL, false);
}

symbol *newVarArray (symbol **tos, char *id, s_array *arr, symbol *curfun, bool ref) {
    return newVar(tos, S_ARRAY, id, arr, curfun, ref);
}

symbol *newProg (symbol **tos, char *id) {
    return newVar(tos, S_PROG, id, NULL, NULL, false);
}

symbol *searchTable (symbol *tos, char *id, symbol *curfun) {
    char var[LEN], fun[LEN];
    int res;

    if (curfun == NULL)
        res = snprintf(var, LEN, "var_%s", id);
    else
        res = snprintf(var, LEN, "funvar_%s_%s", curfun->id, id);

    if (res < 0 || res >= LEN)
        ferr(__LINE__ ,"stable.c searchTable snprintf");

    res = snprintf(fun, LEN, "fun_%s", id);
    if (res < 0 || res >= LEN)
        ferr(__LINE__ ,"stable.c searchTable snprintf");

    while (tos != NULL) {
        if (((strcmp(tos->id, var)) == 0) || (strcmp(tos->id, fun) == 0))
            return tos;
        tos = tos->next;
    }

    return NULL;
}

/**
 * @param gtos Global table of symbols
 * @param curfun Current function symbol (NULL if not in function)
 * @param id Symbol ID to search
 */

symbol *search (symbol *gtos, symbol *curfun, char *id) {
    symbol *s;
    symbol *ftos = NULL;
    if (curfun != NULL)
        ftos = ((fundata *) curfun->fdata)->tos;

    if (ftos != NULL && (s = searchTable(ftos, id, curfun)) != NULL)
        return s;

    return searchTable(gtos, id, curfun);
}

void stablePrint (symbol *tos) {
    while (tos != NULL) {
        fprintf(stdout, "%s: ", tos->id);
        switch (tos->type) {
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
                switch (((fundata *) tos->fdata)->rtype) {
                    case S_INT:
                        fprintf(stdout, "INTEGER\n");
                        break;
                    case S_BOOL:
                        fprintf(stdout, "BOOLEAN\n");
                        break;
                    case S_UNIT:
                        fprintf(stdout, "UNIT\n");
                        break;
                    default:
                        //Others todo
                        break;
                }
                break;
            default:
                //OTHERS TODO
                break;
        }
        // fprintf(stdout, "\n");
        tos = tos->next;
    }
}
