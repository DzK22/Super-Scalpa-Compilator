%{
    #include <stdio.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <math.h>
    #include "../headers/stable.h"
    #include "../headers/quad.h"
    #include "../headers/mips.h"
    #include "../headers/arglist.h"
    #define YYDEBUG 1

    int yyerror (char *s);
    int yylex (void);
    void freeLex (void);
    extern FILE *yyin;

    symbol *stable = NULL;
    quad *all_code = NULL;
    char *progName = NULL;
    int instr_cnt  = 0;

    void testID (symbol *s, char *name) {
        if(s == NULL) {
            char s[LEN];
            snprintf(s, LEN, "cs.y ID \"%s\" not exists", name);
            ferr(s);
        }
    }

    void arithmeticExpression(qop op, symbol **res, quad **quadRes, quad *quad1, symbol *arg1, quad *quad2, symbol *arg2)
    {
        // OK
        symbol *ptr = newTmpInt(&stable, 0);
        quad *q = qGen(op, ptr, arg1, arg2);

        *res = ptr;
        *quadRes = concat(quad1, quad2);
        *quadRes = concat(*quadRes, q);
     }

%}

%union {
    int   ival;
    bool  bval;
    char  *sval;
    stype type;
    qop   op;

    struct {
        struct symbol    *ptr;
        struct quad      *quad;
        struct quad      *ltrue;
        struct quad      *lfalse;
    } gencode; // Pour les expressions

    struct {
        stype type;
        union {
            int  ival;
            bool bval;
            char *sval;
        };
    } cte;

    struct arglist *arglist;
}

%token PROGRAM_ IDENT_ NEWLINE_ END_ WRITE_ BEGIN_ READ_ AFFEC_ INT_ BOOL_ STRING_ UNIT_ VAR_ RETURN_ REF_ IF_ THEN_ ELSE_ WHILE_ DO_ DOTCOMMA_ COMMA_ CTE_ PARLEFT_ PARRIGHT_ BRALEFT_ BRARIGHT_ // common tokens
%token MULT_ DIV_ PLUS_ MINUS_ EXP_ INF_ INF_EQ_ SUP_ SUP_EQ_ EQUAL_ DIFF_ AND_ OR_ XOR_ NOT_ // operators (binary or unary)

%type <sval>     IDENT_
%type <cte>      CTE_
%type <gencode>  expr instr program sequence lvalue m
%type <listDecl> fundecllist  vardecllist fundecl
%type <arglist>  identlist varsdecl
%type <type>     typename atomictype

%right EQUAL_
%nonassoc   INF_EQ_ INF_ SUP_EQ_ SUP_ DIFF_
%left   PLUS_ MINUS_ OR_
%left   MULT_ DIV_ AND_
%right  AFFEC_
%right  EXP_
%left   NOT_

%start  program
%%

program: PROGRAM_ IDENT_ vardecllist fundecllist instr  {
        newVarInt(&stable, $2, 0);
        progName = strdup($2);
        $$.quad  = concat(NULL, $5.quad);
        all_code = $$.quad;
    }
    ;

vardecllist : %empty                         { }
            | varsdecl                       { }
            | varsdecl DOTCOMMA_ vardecllist { }
           ;

varsdecl: VAR_ identlist ':' typename {
             arglistPrint($2);
             switch ($4) {
                 case S_INT:
                    fprintf(stdout, "integer\n");
                    break;
                 case S_BOOL:
                    fprintf(stdout, "boolean\n");
                    break;
                 case S_STRING:
                    fprintf(stdout, "string\n");
                    break;
             }
             arglist *al = $2;

              while (al != NULL) {
                  switch ($4) {
                      case S_BOOL:
                          newVarBool(&stable, al->id, false);
                          break;
                      case S_INT:
                          newVarInt(&stable, al->id, 0);
                          break;
                      case S_STRING:
                          newVarStr(&stable, al->id, "\"\"");
                          break;
                    default:
                        ferr("cs.y varsdecl identlist An arg has wrong type");
                  }

                  al = al->next;
              }
          }
          ;

identlist : IDENT_ {
                $$ = arglistNew(strdup($1), S_NONE, NULL);
            }
         | IDENT_ COMMA_ identlist {
                arglist *al = arglistNew(strdup($1), S_NONE, NULL);
                $$ = arglistConcat(al, $3);
            }
         ;

typename : atomictype {
            $$ = $1;
          }
         ;

atomictype : UNIT_   { $$ = S_UNIT;    }
           | BOOL_   { $$ = S_BOOL;    }
           | INT_    { $$ = S_INT;     }
           | STRING_ { $$ = S_STRING;  }
           ;

fundecllist : %empty                        {  }
            | fundecl DOTCOMMA_ fundecllist       {  }
           ;

/* BOUMBOUM = pour ne pas avoir de conflit bison, a virer quand implementation des fonctions mdr */
fundecl : "BOUMBOUM"                            {}
        ;

instr: lvalue AFFEC_ expr {
                quad *q    = qGen(Q_AFFEC, $1.ptr, $3.ptr, NULL);
                quad *quad = concat($3.quad, q);
                $$.quad    = quad;
                $$.ptr     = $1.ptr;

                if ($1.ptr->type != $3.ptr->type)
                    ferr("cs.y instr: lvalue and expr type differ");
            }
        | RETURN_ expr {}
        | RETURN_ {}
        | IDENT_ PARLEFT_ exprlist PARRIGHT_ {}
        | IDENT_ PARLEFT_ PARRIGHT_ {}
        | BEGIN_ sequence END_ {
                $$.ptr  = $2.ptr;
                $$.quad = $2.quad;
            }
        | BEGIN_ END_ {}
        | READ_ expr {
                quad *q = qGen(Q_READ, $2.ptr, NULL, NULL);
                $$.quad = concat($2.quad, q);
            }
        | WRITE_ expr {
                quad *q = qGen(Q_WRITE, NULL, $2.ptr, NULL);
                $$.quad = concat($2.quad, q);
            }
        | IF_ expr THEN_ m instr m {
                quad *qif   = qGen(Q_IF, NULL, $2.ptr, NULL);
                qif->gtrue  = $4.quad->res;
                qif->gfalse = $6.quad->res;
                qif->gnext  = $6.quad->res;

                $$.quad = concat($2.quad, qif);
                $$.quad = concat($$.quad, $4.quad);
                $$.quad = concat($$.quad, $5.quad);
                $$.quad = concat($$.quad, $6.quad);
            }
        | IF_ expr THEN_ m instr ELSE_ m instr m {
                quad *qif   = qGen(Q_IF, NULL, $2.ptr, NULL);
                qif->gtrue  = $4.quad->res;
                qif->gfalse = $7.quad->res;
                qif->gnext  = $9.quad->res;

                quad *go = qGen(Q_GOTO, qif->gnext, NULL, NULL);

                $$.quad = concat($2.quad, qif);
                $$.quad = concat($$.quad, $4.quad);
                $$.quad = concat($$.quad, $5.quad);
                $$.quad = concat($$.quad, go);
                $$.quad = concat($$.quad, $7.quad);
                $$.quad = concat($$.quad, $8.quad);
                $$.quad = concat($$.quad, $9.quad);
            }
        | WHILE_ m expr DO_ m instr m {
                quad *qif   = qGen(Q_IF, NULL, $3.ptr, NULL);
                qif->gtrue  = $5.quad->res;
                qif->gfalse = $7.quad->res;
                qif->gnext  = $2.quad->res;

                quad *go = qGen(Q_GOTO, qif->gnext, NULL, NULL);

                $$.quad = concat($2.quad, $3.quad);
                $$.quad = concat($$.quad, qif);
                $$.quad = concat($$.quad, $5.quad);
                $$.quad = concat($$.quad, $6.quad);
                $$.quad = concat($$.quad, go);
                $$.quad = concat($$.quad, $7.quad);
            }
      ;

sequence : instr DOTCOMMA_ sequence  {
                $$.quad = concat($1.quad, $3.quad);
                $$.ptr  = $1.ptr;
             }
             | instr DOTCOMMA_ {
                 $$.ptr  = $1.ptr;
                 $$.quad = $1.quad;
             }
             | instr  {
                 $$.ptr  = $1.ptr;
                 $$.quad = $1.quad;
             }
        ;

lvalue: IDENT_ {
                symbol *ptr = search(stable, $1);
                testID(ptr, $1);

                $$.ptr  = ptr;
                $$.quad = NULL;
                free($1);
            }

        | IDENT_ BRALEFT_ exprlist BRARIGHT_ {

            }
      ;

exprlist : expr {}
        |  expr COMMA_ exprlist {}
        ;

expr : CTE_ {
                symbol *ptr;
                switch ($1.type) {
                    case S_INT    : ptr = newTmpInt (&stable, $1.ival);
                                    break;
                    case S_BOOL   : ptr = newTmpBool(&stable, $1.bval);
                                    break;
                    case S_STRING : ptr = newTmpStr (&stable, $1.sval);
                                    free($1.sval);
                                    break;
                    default: ferr("cs.y expr : CTE_ Unknow cte type");
                }

                $$.ptr  = ptr;
                $$.quad = NULL;
                $$.ltrue = NULL;
                $$.lfalse = NULL;
            }
      | PARLEFT_ expr PARRIGHT_ {
                $$.ptr    = $2.ptr;
                $$.quad   = $2.quad;
                $$.ltrue  = $2.ltrue;
                $$.lfalse = $2.lfalse;
            }


            // TODO
      | expr PLUS_ expr {
            if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
              ferr("cs.y expr PLUS expr type error");

            arithmeticExpression(Q_PLUS, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
          }

      | expr MINUS_ expr {
            if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
                ferr("cs.y expr MULT expr type error");

            arithmeticExpression(Q_MINUS, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
          }

       | expr MULT_ expr {
           if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
               ferr("cs.y expr MULT expr type error");
            arithmeticExpression(Q_MULT, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
          }

      | expr DIV_ expr {
            if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
                ferr("cs.y expr MULT expr type error");

            arithmeticExpression(Q_DIV, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
          }

      | expr OR_ m expr {
            if ($1.ptr->type != $4.ptr->type || $1.ptr->type != S_BOOL)
                ferr("cs.y expr OR expr type error");

            symbol *ptr = newTmpBool(&stable, false);
            $$.ptr = ptr;


            complete($1.lfalse, false, $3.quad->res);
            $$.lfalse = $4.lfalse;
            $$.ltrue = concat($1.ltrue, $4.ltrue);

            quad *q = qGen(Q_OR, ptr, $1.ptr, $4.ptr);
            $$.quad = concat($1.quad, $3.quad);
            $$.quad = concat($$.quad, $4.quad);
            $$.quad = concat($$.quad, q);
          }

      | expr AND_ m expr {
            if ($1.ptr->type != $4.ptr->type || $1.ptr->type != S_BOOL)
                ferr("cs.y expr AND expr type error");
            symbol *ptr = newTmpBool(&stable, false);
            $$.ptr = ptr;

            complete($1.ltrue, true, $3.quad->res);
            $$.lfalse = concat($1.lfalse, $4.lfalse);
            $$.ltrue = $4.ltrue;

            quad *q = qGen(Q_AND, ptr, $1.ptr, $4.ptr);
            $$.quad = concat($1.quad, $3.quad);
            $$.quad = concat($$.quad, $4.quad);
            $$.quad = concat($$.quad, q);
          }

      | expr XOR_ m expr {
            if ($1.ptr->type != $4.ptr->type || $1.ptr->type != S_BOOL)
                ferr("cs.y expr XOR expr type error");

            symbol *ptr = newTmpBool(&stable, false);
            $$.ptr = ptr;

            // complete true false etc

            quad *q = qGen(Q_XOR, ptr, $1.ptr, $4.ptr);
            $$.quad = concat($1.quad, $3.quad);
            $$.quad = concat($$.quad, $4.quad);
            $$.quad = concat($$.quad, q);
          }

        | NOT_ expr {
            if ($2.ptr->type != S_BOOL)
                ferr("cs.y expr NOT type error");

            symbol *ptr = newTmpBool(&stable, false);
            $$.ptr = ptr;

            quad *q = qGen(Q_NOT, ptr, $2.ptr, NULL);
            $$.quad = $2.quad;
            $$.quad = concat($$.quad, q);
        }

        | MINUS_ expr {
            if ($2.ptr->type != S_INT)
                ferr("cs.y expr INT type error");

            symbol *ptr = newTmpInt(&stable, 0);
            $$.ptr = ptr;
            quad *q = qGen(Q_MINUS, ptr, $2.ptr, NULL);
            $$.quad = $2.quad;
            $$.quad = concat($$.quad, q);
        }

      | IDENT_ PARLEFT_ exprlist PARRIGHT_ {
                // function call (with parameters)
            }
      | IDENT_ PARLEFT_ PARRIGHT_ {
                // procedure call (no parameters)
            }
      | IDENT_ BRALEFT_ exprlist BRARIGHT_ {
                // array with indexes
            }
      | IDENT_ {
                symbol *ptr = search(stable, $1);
                testID(ptr, $1);

                $$.ptr  = ptr;
                $$.quad = NULL;
                $$.ltrue = NULL;
                $$.lfalse = NULL;
                free($1);
            }
      ;

m : %empty {
          $$.quad = qGen(Q_LABEL, newTmpLabel(&stable), NULL, NULL);
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

    #if YYDEBUG
        /* yydebug = 1; */
    #endif

    yyin = fopen(argv[1], "r");
    yyparse();
    char out[LEN];

    int res = snprintf(out, LEN, "%s.s", *progName ? progName : "out");
    if (res < 0 || res >= LEN) {
        fprintf(stderr, "snprintf error\n");
        return EXIT_FAILURE;
    }

    FILE *output = fopen(out, "w");
    qPrint(all_code);
    getMips(output, stable, all_code);

    freeLex();
    qFree(all_code);
    sFree(stable);

    return EXIT_SUCCESS;
}
