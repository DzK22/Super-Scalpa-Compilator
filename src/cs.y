%{

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../headers/stable.h"
#include <math.h>
int yyerror (char *s);
int yylex (void);

%}

%union {
    char    tid; //token ID
    int     index; //token num
    struct  symbol *label;
    struct {
        struct symbol *res;
        //Quad à rajouter quand gencode
    } c_expr; // Pour les expressions
}

/*Pour tester*/
%union {
    int value;
}

%token PROGRAM ID EOF_ MULT DIV PLUS MINUS EXP INF INF_EQ SUP SUP_EQ EQUAL DIFF AFFEC AND OR XOR NOT
%token <value> INTEGER
%type <value> expr
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

instr   : expr                          { fprintf(stdout, "résultat = %d\n", $1); }
        ;

expr    : '(' expr ')'                  { $$ = $2; }
        | expr PLUS expr                { $$ = $1 + $3; }
        | expr MINUS expr               { $$ = $1 - $3; }
        | expr MULT expr                { $$ = $1 * $3; }
        | expr DIV expr                 { $$ = ($3 != 0) ? $1 / $3 : 0; }
        | expr EXP expr                 { $$ = pow($1, $3); }
        | INTEGER                       { $$ = $1;}
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
