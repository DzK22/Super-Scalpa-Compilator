#include "../headers/opti.h"

#define optideb(s) \
	printf(" ## OPTIDEB :::\t  %-26s ##\n", s);

static char tbuf[LEN];

void optiLoop (quad **code, symbol **gtos) {
	int loops = 0, cnt = 1, res;
	optideb("BEGIN OPTIMIZATION");
	(void) gtos;

	while (cnt > 0) {
		loops ++;
		cnt = 0;

		cnt += optiDeadCode(code);
		cnt += optiVarDuplicate(code);
		cnt += optiArithOps(code);
		res = snprintf(tbuf, LEN, "LOOP [%d] => %2d changes", loops, cnt);
		if (res < 0 || res >= LEN)
			ferr(__LINE__ ,"opti.c optiLoop snprintf");

		optideb(tbuf);
	}
}

int optiDeadCode (quad **code) {
	int cnt = 0;
	quad *q = *code;

	while (q != NULL) {
		// sup code mort
		q = q->next;
	}

	return cnt;
}

int optiVarDuplicate (quad **code) {
	int cnt = 0;
	quad *q = *code;

	while (q != NULL) {
		// sup variables dupliquées
		q = q->next;
	}

	return cnt;
}

int optiAddZero (quad *q) {
	if (q->op != Q_PLUS && q->op != Q_MINUS)
		return 0;
	if (q->argv1->type != S_NONE) {
		switch (q->argv1->type) {
			case S_INT:
				if (q->argv1->ival == 0) {
					q->op = Q_AFFEC;
					q->argv1 = q->argv2;
					q->argv2 = NULL;
					return 1;
				}
				break;
			default:
				return 0;
		}
	}
	if (q->argv2->type != S_NONE) {
		switch (q->argv2->type) {
			case S_INT:
				if (q->argv2->ival == 0) {
					q->op = Q_AFFEC;
					q->argv2 = NULL;
					return 1;
				}
				break;
			default:
				return 0;
		}
	}

	return 0;
}

int optiMultOne (quad *q) {
	if (q->op != Q_MULT && q->op != Q_DIV)
		return 0;
	if (q->argv1->type != S_NONE) {
		switch (q->argv1->type) {
			case S_INT:
				if (q->argv1->ival == 1) {
					q->op = Q_AFFEC;
					q->argv1 = q->argv2;
					q->argv2 = NULL;
					return 1;
				}
				break;
			default:
				return 0;
		}
	}
	if (q->argv2->type != S_NONE) {
		switch (q->argv2->type) {
			case S_INT:
				if (q->argv2->ival == 1) {
					q->op = Q_AFFEC;
					q->argv2 = NULL;
					return 1;
				}
				break;
			default:
				return 0;
		}
	}
	return 0;
}

//je pense il faudra traité tab[i,j] d'une autre manière vu la magouille qu'on a faite avec argv2-calcul d'index pour l'affectation (ou peut etre je me mélange les pinceaux et jdis de la merde)
int optiArithOps (quad **code) {
	int cnt = 0;
	quad *q = *code;

	while (q != NULL) {
		cnt += optiAddZero(q);
		cnt += optiMultOne(q);
		q = q->next;
	}
	return cnt;
}
