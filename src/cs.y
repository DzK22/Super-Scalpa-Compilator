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
    int instr_cnt  = 0;

    void testID (symbol *s, char *name) {
        if (s == NULL) {
            char s[LEN];
            snprintf(s, LEN, "cs.y ID \"%s\" not exists", name);
            ferr(s);
        }
    }
%}

%union {
    char *tid;
    int  val;
    struct symbol *s;
    stype type;
    struct {
        struct symbol   *res;
        struct quad     *code;
    } gencode; // Pour les expressions
}

%token  PROGRAM ID NEWLINE END WRITE INTEGER MULT DIV PLUS MINUS EXP INF INF_EQ SUP SUP_EQ EQUAL DIFF AFFEC AND OR XOR NOT TYPE VAR
%type   <val> INTEGER
%type   <tid> ID
%type   <type> TYPE
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
                if (instr_cnt ++ == 0)
                    $$.code = NULL;

                $$.code = concat($$.code, $2.code);
                all_code = $$.code;
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
                symbol *res;
                if ($4 == S_INTEGER)
                    res = newVarInt(&stable, $2, 0);
                else if ($4 == S_STRING)
                    res = newVarStr(&stable, $2, "");

                $$.res = res;
            }
        | ID AFFEC expr
            {
                symbol *res = search(stable, $1);
                testID(res, $1);

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
                symbol *res = newTmpInt(&stable, 0);
                $$.res      = res;
                quad *q     = qGen(Q_PLUS, res, $1.res, $3.res);
                quad *code  = concat($1.code, $3.code);
                code        = concat(code, q);
                $$.code     = code;
            }
        | expr MINUS expr
            {
                symbol *res = newTmpInt(&stable, 0);
                $$.res      = res;
                quad *q     = qGen(Q_MINUS, res, $1.res, $3.res);
                quad *code  = concat($1.code, $3.code);
                code        = concat(code, q);
                $$.code     = code;
            }
        | expr MULT expr
            {
                symbol *res = newTmpInt(&stable, 0);
                $$.res      = res;
                quad *q     = qGen(Q_MULT, res, $1.res, $3.res);
                quad *code  = concat($1.code, $3.code);
                code        = concat(code, q);
                $$.code     = code;
            }
        | expr DIV expr
            {
                symbol *res = newTmpInt(&stable, 0);
                $$.res      = res;
                quad *q     = qGen(Q_DIV, res, $1.res, $3.res);
                quad *code  = concat($1.code, $3.code);
                code        = concat(code, q);
                $$.code     = code;
            }
        | expr EXP expr
            {
                symbol *res = newTmpInt(&stable, 0);
                $$.res      = res;
                quad *q     = qGen(Q_EXP, res, $1.res, $3.res);
                quad *code  = concat($1.code, $3.code);
                code        = concat(code, q);
                $$.code     = code;
            }
        | INTEGER
            {
                symbol *res = newTmpInt(&stable, $1);
                $$.res      = res;
                $$.code     = NULL;
            }
        | MINUS expr
            {
                symbol *res = newTmpInt(&stable, 0);
                symbol *tmp = newTmpInt(&stable, 0);
                $$.res     = res;
                quad *q    = qGen(Q_MINUS, res, tmp, $2.res);
                quad *code = concat($2.code, q);
                $$.code    = code;
            }
        | ID
            {
                symbol *res = search(stable, $1);
                testID(res, $1);

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
    qPrint(all_code);

    FILE *f_out = fopen("out.s", "w");
    getMips(f_out, stable, all_code);

    freeLex();
    qFree(all_code);
    sFree(stable);

    return EXIT_SUCCESS;
}
