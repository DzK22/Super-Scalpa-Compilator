%{
    #include <stdio.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <math.h>
    #include "../headers/stable.h"
    #include "../headers/quad.h"
    #include "../headers/mips.h"

    int yyerror (char *s);
    int yylex (void);
    void freeLex (void);
    extern FILE *yyin;
    symbol *stable = NULL;
    quad *all_code = NULL;
%}

%union {
    char    *tid;
    int     value;
    struct  symbol *s;
    struct {
        struct symbol   *res;
        struct quad     *code;
    } gencode; // Pour les expressions
}

%token  PROGRAM ID NEWLINE END WRITE INTEGER MULT DIV PLUS MINUS EXP INF INF_EQ SUP SUP_EQ EQUAL DIFF AFFEC AND OR XOR NOT TYPE VAR
%type   <value> INTEGER
%type   <tid> ID
%type   <gencode>  expr program instr
%left   PLUS MINUS
%left   MULT DIV
%right  EXP
%left   NOT

%start  program
%%

program : %empty                        { }
        | program instr NEWLINE
            {
                $$.code = concat(NULL, $2.code);
                all_code = $$.code;
                qPrint(all_code);
                //quad *last = getLast($2.code);
                /*if (last->op != Q_END) {
                    fprintf(stderr, "programm shouldnt end with \"end\" keyword\n");
                    exit(EXIT_FAILURE);
                }*/
            }
        ;

instr   : expr
            {
                $$.res = $1.res;
                $$.code = $1.code;
            }
        | END
            {
                $$.code = qGen(Q_END, NULL, NULL, NULL);
            }
        | WRITE expr
            {
                $$.res  = $2.res;
                quad *q = qGen(Q_WRITE, $$.res, NULL, NULL);
                $$.code = concat($2.code, q);
            }
        | VAR ID ':' TYPE
            {
                symbol *res = sAdd(&stable, $2);
                res->cst    = true;
                res->init   = true;
                res->value  = 0;
                res->type   = INTEGER_;
                $$.res      = res;
            }
        | ID AFFEC expr
            {
                symbol *res = search(stable, $1);

                if (res == NULL) {
                    char s[LEN];
                    snprintf(s, LEN, "cs.y expr ID \"%s\" not exists", $1);
                    ferr(s);
                }

                if (!res->init) {
                    char s[LEN];
                    snprintf(s, LEN, "cs.y expr ID \"%s\" is not init", $1);
                    ferr(s);
                }

                quad *q    = qGen(Q_AFFEC, res, $3.res, NULL);
                quad *code = concat($3.code, q);
                $$.code    = code;
            }
        ;

expr    : '(' expr ')'
            {
                $$.code = $2.code;
                $$.res = $2.res;
            }
        | expr PLUS expr
            {
                symbol *res = newTemp(&stable);
                $$.res      = res;
                quad *q     = qGen(Q_PLUS, res, $1.res, $3.res);
                quad *code  = concat($1.code, $3.code);
                code        = concat(code, q);
                $$.code     = code;
                //qPrint($$.code);
            }
        | expr MINUS expr
            {
                symbol *res = newTemp(&stable);
                $$.res      = res;
                quad *q     = qGen(Q_MINUS, res, $1.res, $3.res);
                quad *code  = concat($1.code, $3.code);
                code        = concat(code, q);
                $$.code     = code;
                //qPrint($$.code);
            }
        | expr MULT expr
            {
                symbol *res = newTemp(&stable);
                $$.res      = res;
                quad *q     = qGen(Q_MULT, res, $1.res, $3.res);
                quad *code  = concat($1.code, $3.code);
                code        = concat(code, q);
                $$.code     = code;
                //qPrint($$.code);
            }
        | expr DIV expr
            {
                symbol *res = newTemp(&stable);
                $$.res      = res;
                quad *q     = qGen(Q_DIV, res, $1.res, $3.res);
                quad *code  = concat($1.code, $3.code);
                code        = concat(code, q);
                $$.code     = code;
                //qPrint($$.code);
            }
        | expr EXP expr
            {
                symbol *res = newTemp(&stable);
                $$.res      = res;
                quad *q     = qGen(Q_EXP, res, $1.res, $3.res);
                quad *code  = concat($1.code, $3.code);
                code        = concat(code, q);
                $$.code     = code;
                //qPrint($$.code);
            }
        | INTEGER
            {
                symbol *res = newCstInt(&stable, $1);
                $$.res      = res;
                $$.code     = NULL;
            }
        | MINUS expr
            {
                symbol *res = newTemp(&stable);
                symbol *tmp = newCstInt(&stable, 0);
                $$.res     = res;
                quad *q    = qGen(Q_MINUS, res, tmp, $2.res);
                quad *code = concat($2.code, q);
                $$.code    = code;
            }
        | ID
            {
                symbol *res = search(stable, $1);

                if (res == NULL) {
                    char s[LEN];
                    snprintf(s, LEN, "cs.y expr ID \"%s\" not exists", $1);
                    ferr(s);
                }

                if (!res->init) {
                    char s[LEN];
                    snprintf(s, LEN, "cs.y expr ID \"%s\" is not init", $1);
                    ferr(s);
                }

                $$.res = res;
                $$.code = NULL;
                free($1);
            }
        ;
%%

int yyerror (char *s)
{
    ferr(s);
}

int yywrap (void)
{
  return 1;
}

int main (int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <scalpa_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f_in = fopen(argv[1], "r");
    if (f_in == NULL)
        ferr("cs.y main fopen");

    yyin = f_in;
    yyparse();

    FILE *f_out = fopen("out.s", "w");
    getMips(f_out, stable, all_code);

    freeLex();
    qFree(all_code);
    sFree(stable);

    return EXIT_SUCCESS;
}
