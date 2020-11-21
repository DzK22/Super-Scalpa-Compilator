#include "../headers/stable.h"
static int nsym = 0;

void sFree (symbol *s) {
    symbol *cur;
    while (s != NULL) {
        cur = s;
        s = s->next;
        if (cur) {
            if (cur->id)
                free(cur->id);
            if (cur->type == STRING_);
                free(cur->string);

        }
        free(cur);
    }
}

symbol *newTemp (symbol **stable) {
    symbol *ns = malloc(sizeof(struct symbol));
    if (ns == NULL) {
        fprintf(stderr, "malloc error\n");
        return NULL;
    }
    ns->type = INTEGER_;
    ns->cst = false;
    ns->value = 0;
    ns->id = malloc(LEN);
    if (ns->id == NULL) {
        fprintf(stderr, "malloc error\n");
        return NULL;
    }
    int res = snprintf(ns->id, LEN, "temp_%d", nsym++);
    if (res < 0 || res >= LEN) {
        fprintf(stderr, "snprintf error\n");
        return NULL;
    }
    ns->next = NULL;
    symbol *cur = *stable;
    if (cur == NULL)
        *stable = ns;
    else {
        while (cur->next != NULL)
            cur = cur->next;
        cur->next = ns;
    }
    return ns;
}

symbol *newCstInt (symbol **stable, int cst) {
    symbol *ns = malloc(sizeof(struct symbol));
    if (ns == NULL) {
        fprintf(stderr, "malloc error\n");
        return NULL;
    }
    ns->type = INTEGER_;
    ns->cst = true;
    ns->value = cst;
    ns->id = malloc(LEN);
    if (ns->id == NULL) {
        fprintf(stderr, "malloc error\n");
        return NULL;
    }
    int res;
    if (cst >= 0)
        res = snprintf(ns->id, LEN, "cst_%d", cst);
    else
        res = snprintf(ns->id, LEN, "neg_cst_%d", cst * (-1));
    ns->next = NULL;
    symbol *cur = *stable;
    if (cur == NULL)
        *stable = ns;
    else {
        while (cur->next != NULL)
            cur = cur->next;
        cur->next = ns;
    }
    return ns;
}

symbol *newCstString (symbol **stable, char *cst) {
    symbol *ns = malloc(sizeof(struct symbol));
    if (ns == NULL) {
        fprintf(stderr, "malloc error\n");
        return NULL;
    }
    ns->cst = false;
    ns->type = STRING_;
    if ((ns->string = strdup(cst)) == NULL) {
        fprintf(stderr, "strdup error\n");
        return NULL;
    }
    ns->id = malloc(LEN);
    if (ns->id == NULL) {
        fprintf(stderr, "malloc error\n");
        return NULL;
    }
    int res = snprintf(ns->id, LEN, "string_%d", nsym++);
    if (res < 0 || res >= LEN) {
        fprintf(stderr, "snprintf error\n");
        return NULL;
    }
    ns->next = NULL;
    symbol *cur = *stable;
    if (cur == NULL)
        *stable = ns;
    else {
        while (cur->next != NULL)
            cur = cur->next;
        cur->next = ns;
    }
    return ns;
}

symbol *search (symbol *s, char *id) {
    symbol *cur = s;
    while (cur != NULL && strcmp(cur->id, id)) {
        cur = cur->next;
    }
    return cur;
}
