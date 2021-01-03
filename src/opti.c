#include "../headers/opti.h"

#define optideb(s) \
	printf(" ## OPTIDEB :::\t  %-26s ##\n", s);

static char tbuf[LEN];

void optiLoop (quad **code, symbol **gtos) {
	int loops = 0, cnt = 1, res;
	optideb("BEGIN OPTIMIZATION");

	while (cnt > 0 && loops < 20) { // max 20 loops => security for no infinite loop
		loops ++;
		cnt = 0;

		cnt += optiDuplicateCst(code, gtos);
		cnt += optiArithOp(code, gtos);

		res = snprintf(tbuf, LEN, "LOOP [%d] => %2d changes", loops, cnt);
		if (res < 0 || res >= LEN)
			ferr("optiLoop snprintf");

		optideb(tbuf);
	}
}

int optiDuplicateCst (quad **code, symbol **tos) {
	int cnt = 0;
	quad *q;
	symbol *s = *tos;
	symbol *s1, *s2, *s3, *s4, *s5, *s6;
	list *l, *l1;
	bool changed; // SECURITY => don't remove the var if no change has been found

	while (s != NULL) {
		if (!optiCheckModified(*code, s)) {
			s1 = *tos;
			s4 = NULL;

			while (s1 != NULL) {
				if (s != s1 && sameTypeValue(s, s1) && !optiCheckModified(*code, s1)) {
					// replace all quad which use s1 by s and delete s1
					q = *code;
					changed = false;

					while (q != NULL) {

						if ((q->op == Q_FUNCALL && q->argv2 != NULL)
								|| (q->op == Q_AFFEC && q->argv2 != NULL)
								|| (q->op == Q_READ && q->argv1 != NULL)) {

							if (q->op == Q_READ) {
								s2 = q->argv1;
								s3 = q->argv1;
							} else {
								s2 = q->argv2;
								s3 = q->argv2;
							}

							while (s2 != NULL) {
								if (strcmp(s1->id, s2->id) == 0) {
									// there are one or more cst with same value with s, remake args from the beginning
									changed = true;
									l = NULL;

									while (s3 != NULL) {
										if (strcmp(s1->id, s3->id) == 0) {
											l1 = listNew(NULL, s);
											cnt ++;
										} else
											l1 = listNew(NULL, s3);

										l = listConcat(l, l1);
										s3 = s3->next;
									}

									s6 = listToSymlist(l);

									if (q->op == Q_READ) {
										symListFree(q->argv1);
										q->argv1 = s6;
									} else {
										symListFree(q->argv2);
										q->argv2 = s6;
									}

									listFree(l);
									break;
								}

								s2 = s2->next;
							}
						}

						if (q->argv1 != NULL && strcmp(s1->id, q->argv1->id) == 0) {
							q->argv1 = s;
							changed = true;
							cnt ++;
						}
						if (q->argv2 != NULL && strcmp(s1->id, q->argv2->id) == 0) {
							q->argv2 = s;
							changed = true;
							cnt ++;
						}

						q = q->next;
					}

					if (changed) {
						// delete s1
						s5 = s1;
						s1 = s1->next;
						sDel(s4, s5);

						cnt ++;
						continue;
					}
				}

				s4 = s1;
				s1 = s1->next;
			}
		}

		s = s->next;
	}

	return cnt;
}

int zeroAdd (quad *q) {
	if (q->op != Q_PLUS && q->op != Q_MINUS)
		return 0;
	symbol *argv1 = q->argv1, *argv2 = q->argv2;
	if (argv1->is_cst) {
		if (argv1->ival == 0) {
			q->op = Q_AFFEC;
			q->argv1 = argv2;
			q->argv2 = NULL;
			return 1;
		}
	}
	if (argv2->is_cst) {
		if (argv2->ival == 0) {
			q->op = Q_AFFEC;
			q->argv2 = NULL;
			return 1;
		}
	}
	return 0;
}

int oneMult (quad *q) {
	if (q->op != Q_MULT && q->op != Q_DIV)
		return 0;
	symbol *a1 = q->argv1, *a2 = q->argv2;
	if (a1->is_cst) {
		if (a1->ival == 1) {
			q->op = Q_AFFEC;
			q->argv1 = a2;
			q->argv2 = NULL;
			return 1;
		}
		if ((q->op == Q_MULT || q->op == Q_DIV) && a1->ival == 0) {
			q->op = Q_AFFEC;
			q->argv2 = NULL;
			return 1;
		}
	}
	if (a2->is_cst) {
		if (a2->ival == 1) {
			q->op = Q_AFFEC;
			q->argv2 = NULL;
			return 1;
		}
		if (q->op == Q_MULT && a2->ival == 0) {
			q->op = Q_AFFEC;
			q->argv1 = a2;
			q->argv2 = NULL;
			return 1;
		}
	}
	return 0;
}

int expArith (quad *q) {
	if (q->op != Q_EXP)
		return 0;
	symbol *a1 = q->argv1, *a2 = q->argv2;

	if (a1->is_cst) {
		if (a1->ival == 0 || a1->ival == 1) {
			q->op = Q_AFFEC;
			q->argv2 = NULL;
			return 1;
		}
	}

	if (a2->is_cst && a2->ival == 1) {
		q->op = Q_AFFEC;
		q->argv2 = NULL;
		return 1;
	}

	return 0;
}

int optiArithOp (quad **code, symbol **tos) {
	quad *q = *code;
	(void)tos;
	int cnt = 0;

	while (q != NULL) {
		cnt += zeroAdd(q);
		cnt += oneMult(q);
		cnt += expArith(q);
		q = q->next;
	}

	return cnt;
}

bool optiCheckModified (quad *code, symbol *s) {
	list *al;
	symbol *args;

	while (code != NULL) {
		if (code->res != NULL && strcmp(s->id, code->res->id) == 0)
			return true;

		// if passed in a funcall which take it as a reference, return true
		if (code->op == Q_FUNCALL) {
			args = code->argv2;
			al   = code->argv1->fdata->al;

			while (args != NULL && al != NULL) {
				if (al->sym->ref && strcmp(args->id, s->id) == 0)
					return true;

				args = args->next;
				al = al->next;
			}
		}

		code = code->next;
	}

	return false;
}

bool sameTypeValue (symbol *s1, symbol *s2) {
	if (s1 == NULL || s2 == NULL)
		return false;
	if (s1->type != s2->type)
		return false;
	if (s1->type != S_INT && s1->type != S_BOOL && s1->type != S_STRING)
		return false;

	switch (s1->type) {
		case S_INT    : return s1->ival == s2->ival;
		case S_BOOL   : return s1->bval == s2->bval;
		case S_STRING : return strcmp(s1->sval, s2->sval) == 0;
		default       : return false;
	}
}
