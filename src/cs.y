%{

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../headers/stable.h"
#include "../headers/quad.h"
#include "../headers/mips.h"
#include <math.h>
int yyerror (char *s);
int yylex (void);
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

%token PROGRAM ID EOF_ INTEGER MULT DIV PLUS MINUS EXP INF INF_EQ SUP SUP_EQ EQUAL DIFF AFFEC AND OR XOR NOT
%type <value> INTEGER
%type <tid> ID
%type <gencode>  expr program instr
%left PLUS MINUS
%left MULT DIV
%right EXP
%left NOT

%start program
%%

program : %empty                        { }
        | program instr EOF_
        {
            $$.code = NULL;
            $$.code = concat($$.code, $2.code);
            all_code = $$.code;
            return 0;
        }
        | program EOF_
        {
            $$.code = NULL;
            $$.code = concat($$.code, $1.code);
            all_code = $$.code;
            return 0;
        }
        ;

instr   : expr                          {$$.res = $1.res; $$.code = $1.code; }
        ;

expr    : '(' expr ')'
        {
            $$.code = $2.code;
            $$.res = $2.res;
        }
        | expr PLUS expr
        {
            symbol *res = newTemp(&stable);
            $$.res = res;
            quad *q = qGen(Q_PLUS, res, $1.res, $3.res);
            quad *code = concat($1.code, $3.code);
            code = concat(code, q);
            $$.code = code;
            qPrint($$.code);
        }
        | expr MINUS expr
        {
            symbol *res = newTemp(&stable);
            $$.res = res;
            quad *q = qGen(Q_MINUS, res, $1.res, $3.res);
            quad *code = concat($1.code, $3.code);
            code = concat(code, q);
            $$.code = code;
            qPrint($$.code);
        }
        | expr MULT expr
        {
            symbol *res = newTemp(&stable);
            $$.res = res;
            quad *q = qGen(Q_MULT, res, $1.res, $3.res);
            quad *code = concat($1.code, $3.code);
            code = concat(code, q);
            $$.code = code;
            qPrint($$.code);
        }
        | expr DIV expr
        {
            symbol *res = newTemp(&stable);
            $$.res = res;
            quad *q = qGen(Q_DIV, res, $1.res, $3.res);
            quad *code = concat($1.code, $3.code);
            code = concat(code, q);
            $$.code = code;
            qPrint($$.code);
        }
        | expr EXP expr
        {
            symbol *res = newTemp(&stable);
            $$.res = res;
            quad *q = qGen(Q_EXP, res, $1.res, $3.res);
            quad *code = concat($1.code, $3.code);
            code = concat(code, q);
            $$.code = code;
            qPrint($$.code);
        }
        | INTEGER
        {
            symbol *res = newTemp(&stable);
            res->cst = true;
            res->value = $1;
            $$.res = res;
            $$.code = NULL;
        }
        ;
        | ID
        {
            symbol *res = search(stable, $1);
            if (res == NULL) {
                fprintf(stderr, "ID %s doesnt exist\n", $1);
                exit(EXIT_FAILURE);
            }
            if (!res->init) {
                fprintf(stderr, "token with ID %s doesnt init\n", $1);
                exit(EXIT_FAILURE);
            }
            $$.res = res;
            $$.code = NULL;
            free($1);
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

int main (int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage %s scalpa_file\n", argv[0]);
        return EXIT_FAILURE;
    }
    FILE *f_in = fopen(argv[1], "r");
    if (f_in == NULL) {
        fprintf(stderr, "fopen error\n");
        return EXIT_FAILURE;
    }
    yyin = f_in;
    yyparse();
    FILE *f_out = fopen("out.s", "w");
    getMips(f_out, stable, all_code);
    return EXIT_SUCCESS;
}
