#include "../headers/stable.h"
#include "../headers/list.h"

symbol *sAlloc (void) {
    symbol *ns = malloc(sizeof(struct symbol));
    if (ns == NULL)
        ferr("sAlloc malloc");

    ns->id   = NULL;
    ns->tmp  = false;
    ns->type = S_NONE;
    ns->next = NULL;
    ns->sval = NULL;
    ns->is_cst = false;

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

//Juste Free l'id et sval (si string) quand hashTable sera prête
void sFree (symbol *s) {
    symbol *prev;

    while (s != NULL) {
        prev = s;
        s = s->next;
        if (prev) {
            if (prev->id)
                free(prev->id);
            if (prev->type == S_STRING)
                free(prev->sval);
            if (prev->type == S_ARRAY) {
                freeDimProp(prev->arr->dims);
                free(prev->arr);
            }
            if (prev->type == S_FUNCTION) {
                listFree(prev->fdata->al);
                sFree(prev->fdata->tos);
                free(prev->fdata);
            }
            free(prev);
        }
    }
}

void sDel (symbol *sPrev, symbol *s) {
    if (s == NULL)
        ferr("sDel s is NULL");

    if (sPrev != NULL)
        sPrev->next = s->next;

    if (s->type == S_STRING)
        free(s->sval);
    else if (s->type == S_ARRAY) {
        freeDimProp(s->arr->dims);
        free(s->arr);
    } else if (s->type == S_FUNCTION) {
        listFree(s->fdata->al);
        sFree(s->fdata->tos);
        free(s->fdata);
    }

    free(s->id);
    free(s);
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
            ferr("newVar var ID already exists");

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
        ferr("newVar tid snprintf");

    symbol *nt = sAdd(tos);
    if ((nt->id = strdup(tid)) == NULL)
        ferr("newVar strdup data ID");

    nt->type = type;
    nt->tmp  = isTmp;
    nt->ref  = ref;

    switch (type) {
        case S_INT:
            if (!ref)
                nt->ival = *((int32_t *) data);
            break;
        case S_BOOL:
            if (!ref)
                nt->bval = *((int8_t *) data);
            break;
        case S_STRING:
            if ((nt->sval = strdup((char *) data)) == NULL)
                ferr("newVar strdup data");
            break;
        case S_LABEL:
            if ((nt->sval = strdup(tid)) == NULL)
                ferr("newVar strdup data");
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
            ferr("newVar unknow type");
    }

    return nt;
}

// Helpers functions which call newVar

symbol *newTmpInt (symbol **tos, int32_t val) {
    return newVar(tos, S_INT, NULL, &val, NULL, false);
}

symbol *newTmpStr (symbol **tos, char *str) {
    return newVar(tos, S_STRING, NULL, str, NULL, false);
}

symbol *newTmpBool (symbol **tos, int8_t bol) {
    return newVar(tos, S_BOOL, NULL, &bol, NULL, false);
}

symbol *newTmpLabel (symbol **tos) {
    return newVar(tos, S_LABEL, NULL, NULL, NULL, false);
}

symbol *newVarInt (symbol **tos, char *id, int32_t val, symbol *curfun, bool ref) {
    return newVar(tos, S_INT, id, &val, curfun, ref);
}

symbol *newVarStr (symbol **tos, char *id, char *str, symbol *curfun) {
    return newVar(tos, S_STRING, id, str, curfun, false);
}

symbol *newVarBool (symbol **tos, char *id, int8_t bol, symbol *curfun, bool ref) {
    return newVar(tos, S_BOOL, id, &bol, curfun, ref);
}

symbol *newVarFun (symbol **tos, char *id) {
    fundata *fdata = malloc(sizeof(fundata));
    if (fdata == NULL)
        ferr("malloc error");

    fdata->al    = NULL;
    fdata->rtype = S_NONE;
    fdata->tos   = NULL;
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
        ferr("searchTable snprintf");

    res = snprintf(fun, LEN, "fun_%s", id);
    if (res < 0 || res >= LEN)
        ferr("searchTable snprintf");

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
        ftos = curfun->fdata->tos;

    if (ftos != NULL && (s = searchTable(ftos, id, curfun)) != NULL)
        return s;

    return searchTable(gtos, id, curfun);
}

void stablePrintAll (symbol *tos) {
    printf("\n ### TOS GLOBAL ###\n");
    stablePrint(tos);

    while (tos != NULL) {
        if (tos->type == S_FUNCTION) {
            printf("\n ### TOS OF FUNCTION %s ###\n", tos->id);
            stablePrint(tos->fdata->tos);
        }
        tos = tos->next;
    }

    printf("\n");
}

void stablePrint (symbol *tos) {
    while (tos != NULL) {
        printf("%s\t ", tos->id);

        switch (tos->type) {
            case S_INT:
                printf("INTEGER\t");
                printf("%d\t",tos->ival) ;
                break;
            case S_BOOL:
                printf("BOOLEAN\t");
                printf("%d\t",tos->bval) ;
                break;
            case S_ARRAY:
                printf("ARRAY\t"); // ajouter type de valeurs quand tableaux seront faits
                switch (tos->arr->type) {
                    case S_INT:
                        printf("INTEGER\t");
                        break;
                    case S_BOOL:
                        printf("BOOLEAN\t");
                        break;
                    default:
                        //Others todo
                        break;
                }
                printf("NbDims %d\t",tos->arr->ndims) ;
                while (tos->arr->dims != NULL){
                    printf("[%d .. %d]",tos->arr->dims->min,tos->arr->dims->max );
                    tos->arr->dims = tos->arr->dims->next ;
                }

                printf("\n");

                break;
            case S_STRING:
                printf("STRING\t");
                printf("%s\n",tos->sval) ;

                break;
            case S_PROG:
                printf("PROGRAM NAME\n");
                break;
            case S_FUNCTION:
                printf("Function : returns => ");
                switch (tos->fdata->rtype) {
                    case S_INT:
                        printf("INTEGER\t");
                        break;
                    case S_BOOL:
                        printf("BOOLEAN\t");
                        break;
                    case S_UNIT:
                        printf("UNIT\t");
                        break;
                    default:
                        //Others todo
                        break;
                }

                break;
            default:
                break;
        }

        tos = tos->next;
    }
}
