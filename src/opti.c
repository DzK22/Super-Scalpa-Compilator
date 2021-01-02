#include "../headers/opti.h"

#define optideb(s) \
	printf(" ## OPTIDEB :::\t  %-26s ##\n", s);

static char tbuf[LEN];

void optiLoop (quad **code, symbol **gtos) {
	int loops = 0, cnt = 1, res;
	optideb("BEGIN OPTIMIZATION");

	while (cnt > 0) {
		loops ++;
		cnt = 0;

		cnt += optiDeadCode(code, gtos);
		/* cnt += optiDuplicateCst(code, gtos); */

		res = snprintf(tbuf, LEN, "LOOP [%d] => %2d changes", loops, cnt);
		if (res < 0 || res >= LEN)
			ferr("optiLoop snprintf");

		optideb(tbuf);
	}
}

int optiDeadCode (quad **code, symbol **tos) {
	int cnt = 0;
	quad *q = *code;
	(void) tos;

	while (q != NULL) {
		// sup code mort
		q = q->next;
	}

	return cnt;
}

int optiDuplicateCst (quad **code, symbol **tos) {
	int cnt = 0;
	quad *q;
	symbol *s = *tos;
	symbol *s1, *s2, *s3, *s4, *s5, *s6;
	list *l, *l1;

	while (s != NULL) {
		if (!optiCheckModified(*code, s)) {
			s1 = *tos;
			s4 = NULL;

			while (s1 != NULL) {
				if (s != s1 && sameTypeValue(s, s1) && !optiCheckModified(*code, s1)) {
					// replace all quad which use s1 by s and delete s1
					q = *code;

					while (q != NULL) {
						if (q->op == Q_FUNCALL
						|| (q->op == Q_AFFEC && (q->res->type == S_ARRAY || q->argv1->type == S_ARRAY))
						|| (q->op == Q_READ && q->res->type == S_ARRAY)) {
							if (q->op == Q_READ)
								s2 = q->argv1;
							else
								s2 = q->argv2;

							while (s2 != NULL) {
								if (sameTypeValue(s1, s2) && !optiCheckModified(*code, s2)) {
									// there are one or more cst with same value with s, remake args from the beginning
									s3 = q->argv2;
									l = NULL;

									while (s3 != NULL) {
										if (sameTypeValue(s1, s3)) {
											l1 = listNew(NULL, s);
											cnt ++;
										} else
											l1 = listNew(NULL, s3);

										l = listConcat(l, l1);
										s3 = s3->next;
									}

									s6 = q->op == Q_READ ? q->argv1 : q->argv2;
									symListFree(s6);
									s6 = listToSymlist(l);
									listFree(l);
									break;
								}

								s2 = s2->next;
							}
						} else {
							if (sameTypeValue(s1, q->argv1) && !optiCheckModified(*code, q->argv1)) {
								q->argv1 = s;
								cnt ++;
							}
							if (sameTypeValue(s1, q->argv2) && !optiCheckModified(*code, q->argv2)) {
								q->argv2 = s;
								cnt ++;
							}
						}

						q = q->next;
					}

					// delete s1
					s5 = s1;
					s1 = s1->next;
					sDel(s4, s5);
					continue;
				}

				s4 = s1;
				s1 = s1->next;
			}

		}

		s = s->next;
	}

	return cnt;
}

bool optiCheckModified (quad *code, symbol *s) {
	symbol *smtp;
	/* printf("optiCheckModified pour %s = ", s->id); */

	while (code != NULL) {
		if (code->res == s) {
			/* printf("true\n"); */
			return true;
		}

		if (code->op == Q_FUNCALL
				|| (code->op == Q_AFFEC && (code->res->type == S_ARRAY || code->argv1->type == S_ARRAY))
				|| (code->op == Q_READ && code->res->type == S_ARRAY)) {
			if (code->op == Q_READ)
				smtp = code->argv1;
			else
				smtp = code->argv2;

			while (smtp != NULL) {
				if (smtp == s) {
					/* printf("true\n"); */
					return true;
				}

				smtp = smtp->next;
			}
		}

		code = code->next;
	}

	/* printf("false\n"); */
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
