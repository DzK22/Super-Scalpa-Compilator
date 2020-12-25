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

		res = snprintf(tbuf, LEN, "LOOP [%d] => %2d changes", loops, cnt);
		if (res < 0 || res >= LEN)
			ferr("opti.c optiLoop snprintf");

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
