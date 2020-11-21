%{

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../headers/stable.h"
#include "../headers/quad.h"
#include <math.h>
int yyerror (char *s);
int yylex (void);
symbol *stable = NULL;

%}

%union {
    char    *tid;
    int     value;
    struct  symbol *s;
    struct {
        struct symbol   *res;
        struct quad     *code;
    } expr; // Pour les expressions
}

%token PROGRAM ID EOF_ INTEGER MULT DIV PLUS MINUS EXP INF INF_EQ SUP SUP_EQ EQUAL DIFF AFFEC AND OR XOR NOT
%type <value> INTEGER
%type <tid> ID
%type <expr>  expr
%left PLUS MINUS
%left MULT DIV
%right EXP
%left NOT

%start program
%%

program : %empty                        { }
        | program instr EOF_            { }
        | program EOF_                  { }
        ;

instr   : expr                          { fprintf(stdout, "résultat = %d\n", $1.res->value); }
        ;

expr    : '(' expr ')'
        {
            $$.code = $2.code;
            $$.res = $2.res;
        }
        | expr PLUS expr
        {
            $$.res = newTemp(&stable);
            $$.code = NULL;
            quad *q = qGen(Q_PLUS, $1.res, $3.res, $$.res);
            $$.code = concat($$.code, $1.code);
            $$.code = concat($$.code, $3.code);
            $$.code = concat($$.code, q);
            qPrint($$.code);
        }
        | expr MINUS expr
        {
            $$.res = newTemp(&stable);
            $$.code = NULL;
            quad *q = qGen(Q_MINUS, $1.res, $3.res, $$.res);
            $$.code = concat($$.code, $1.code);
            $$.code = concat($$.code, $3.code);
            $$.code = concat($$.code, q);
        }
        | expr MULT expr
        {
            $$.res = newTemp(&stable);
            $$.code = NULL;
            quad *q = qGen(Q_MULT, $1.res, $3.res, $$.res);
            $$.code = concat($$.code, $1.code);
            $$.code = concat($$.code, $3.code);
            $$.code = concat($$.code, q);
        }
        | expr DIV expr
        {
            $$.res = newTemp(&stable);
            $$.code = NULL;
            quad *q = qGen(Q_DIV, $1.res, $3.res, $$.res);
            $$.code = concat($$.code, $1.code);
            $$.code = concat($$.code, $3.code);
            $$.code = concat($$.code, q);
        }
        | expr EXP expr
        {
            $$.res = newTemp(&stable);
            $$.code = NULL;
            quad *q = qGen(Q_EXP, $1.res, $3.res, $$.res);
            $$.code = concat($$.code, $1.code);
            $$.code = concat($$.code, $3.code);
            $$.code = concat($$.code, q);
        }
        | INTEGER
        {
            char id[LEN];
            int res;
            if ($1 >= 0)
                res = snprintf(id, LEN, "cst_%d", $1);
            else
                res = snprintf(id, LEN, "neg_cst_%d", $1 * (-1));
            if (res < 0 || res >= LEN)
                exit(1);
            struct symbol *s = search(stable, id);
            if (s == NULL)
                s = newCstInt(&stable, $1);
            $$.res = s;
        }
        ;

%%

int yyerror (char *s)
{
  fprintf(stderr, "Error: %s\n", s);
  exit(1);
}

int yywrap (void)
{
  return 1;
}
