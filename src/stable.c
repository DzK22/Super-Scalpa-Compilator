#include "../headers/stable.h"

symbol *sAlloc () {
    symbol *ns = malloc(sizeof(struct symbol));
    if (ns == NULL) {
        fprintf(stderr, "malloc error\n");
        return NULL;
    }

    ns->id     = NULL;
    ns->cst    = false;
    ns->init   = false;
    ns->value  = 0;
    ns->string = NULL;
    ns->next   = NULL;
    return ns;
}

symbol *sAdd (symbol **stable, char *id) {
    if (*stable == NULL) {
        *stable = sAlloc();
        if (*stable == NULL)
            return NULL;

        if (((*stable)->id = strdup(id)) == NULL) {
            fprintf(stderr, "strdup error\n");
            return NULL;
        }

        (*stable)->init = false;
        return *stable;

    } else {
        symbol *cur = *stable;
        while (cur->next != NULL)
            cur = cur->next;

        cur->next = sAlloc();
        if (cur->next == NULL)
            return NULL;

        if ((cur->next->id = strdup(id)) == NULL) {
            fprintf(stderr, "strdup error\n");
            return NULL;
        }

        cur->next->init = false;
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

        if (prev->type == STRING_)
            free(prev->string);

        free(prev);
    }
}

symbol *newTemp (symbol **stable) {
    static int nsym = 0;
    char tid[LEN];
    int res = snprintf(tid, LEN, "temp_%d", nsym++);

    if (res < 0 || res >= LEN) {
        fprintf(stderr, "snprintf error\n");
        return NULL;
    }

    return sAdd(stable, tid);
}

symbol *newCstInt (symbol **stable, int cst) {
    symbol *nt = newTemp(stable);
    if (nt == NULL)
        return NULL;

    nt->cst   = true;
    nt->init  = true;
    nt->value = cst;

    return nt;
}

symbol *newCstString (symbol **stable, char *cst) {
    symbol *nt = newTemp(stable);
    nt->cst    = true;
    nt->init   = true;

    if ((nt->string = strdup(cst)) == NULL)
        ferr("stable.c newCstString strdup");

    return nt;
}

symbol *search (symbol *s, char *id) {
    while (s != NULL) {
        if (strcmp(s->id, id) == 0)
            return s;
        s = s->next;
    }

    return NULL;
}
