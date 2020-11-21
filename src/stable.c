#include "../headers/stable.h"

symbol *sAlloc (void) {
    symbol *ns = malloc(sizeof(struct symbol));
    if (ns == NULL) {
        fprintf(stderr, "malloc error\n");
        return NULL;
    }
    ns->id = NULL;
    ns->type = NONE_;
    ns->value.v_empty = NULL;
    ns->next = NULL;
    ns->cst = false;
    return ns;
}

void sFree (symbol *s) {
    if (s != NULL) {
        free(s->id);
        free(s);
    }
}

symbol *sAdd (symbol **stable, char *id, stype type) {
    char name[LEN];
    int res = snprintf(name, LEN, "var_%s", id);
    if (res < 0 || res >= LEN) {
        fprintf(stderr, "snprintf error\n");
        return NULL;
    }
    if (*stable == NULL) {
        if ((*stable = sAlloc()) == NULL)
            return NULL;
        if (((*stable)->id = strdup(name)) == NULL)
            return NULL;
        (*stable)->type = type;
        return *stable;
    }
    else {
        symbol *cur = *stable;
        do {
            cur = cur->next;
        } while (cur->next != NULL);
        if ((cur->next = sAlloc()) == NULL)
            return NULL;
        if ((cur->next->id = strdup(name)) == NULL)
            return NULL;
        cur->next->type = type;
        return cur->next;
    }
}
symbol *sNewTemp (symbol **stable, stype type) {
    static int nsym = 0;
    char name[LEN];
    int res = snprintf(name, LEN, "tmp_%d", nsym);
    if (res < 0 || res >= LEN) {
        fprintf(stderr, "snprintf error\n");
        return NULL;
    }
    nsym++;
    return sAdd(stable, name, type);
}

symbol *sNewCstInt (symbol **stable, int cst) {
    symbol *ns = sNewTemp(stable, INTEGER_);
    if (ns == NULL)
        return NULL;
    ns->cst = true;
    ns->value.v_int = cst;
    return ns;
}

symbol *sNewStringInt (symbol **stable, char *cst) {
    symbol *ns = sNewTemp(stable, STRING_);
    if (ns == NULL)
        return NULL;
    ns->cst = true;
    if ((ns->value.v_string = strdup(cst)) == NULL)
        return NULL;
    return ns;
}
