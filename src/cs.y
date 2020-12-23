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

    int yyerror  (char *s);
    int yylex    (void);
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

    void printRange (t_range *list) {
        t_range *cur = list;
        while (cur != NULL) {
                fprintf(stdout, "min:%d[]max:%d[]ndim:%d\n", cur->min, cur->max, cur->ndim);
            cur = cur->next;
        }
    }

    void arithmeticExpression (qop op, symbol **res, quad **quadRes, quad *quad1, symbol *arg1, quad *quad2, symbol *arg2) {
        // OK
        symbol *ptr = newTmpInt(&stable, 0);
        quad *q     = qGen(op, ptr, arg1, arg2);

        *res        = ptr;
        *quadRes    = concat(quad1, quad2);
        *quadRes    = concat(*quadRes, q);
     }

    void booleanExpression (qop op, symbol **res, quad **quadRes, quad *quad1, symbol *arg1, quad *quad2, symbol *arg2) {
        // OK
        symbol *ptr  = newTmpInt(&stable, 0);
        quad *q      = qGen(op, ptr, arg1, arg2);

        *res         = ptr;
        *quadRes     = concat(quad1, quad2);
        *quadRes     = concat(*quadRes, q);
        (*res)->type = S_BOOL;
     }

    // el = NULL for procedure
    void funcallExpression (quad **quadRes, symbol **symRes, char *id, exprlist *el) {
        symbol *fs = search(stable, id);
        testID(fs, id);

        if (fs->type != S_FUNCTION)
            ferr("cs.y funcallExpression symbol is not a function");

        symbol *res;
        fundata *fdata = (fundata *) fs->fdata;

        switch (fdata->rtype) {
            case S_INT  : res = newTmpInt(&stable, 0)      ; break ;
            case S_BOOL : res = newTmpBool(&stable, false) ; break ;
            case S_UNIT : res = NULL                       ; break ;
            default: ferr("cs.y funcallExpression wrong return type");
        }

        symbol *slist = NULL;
        if (el) { // args présent
            // convertir arglist en nouv liste de symbol car gGen ne prend que des symbol * en arg
            slist = arglistToSymlist(el->al);
        }

        quad *q  = qGen(Q_FUNCALL, res, fs, slist);
        *quadRes = NULL;

        if (el)
            *quadRes = concat(*quadRes, el->quad);
        *quadRes = concat(*quadRes, q);
        *symRes  = NULL;
     }
%}

%union {
    int   ival;
    bool  bval;
    char  *sval;
    char  signe;
    stype type;
    qop   op;

    struct {
        struct symbol *ptr;
        struct quad   *quad;
        struct quad   *ltrue;
        struct quad   *lfalse;
    } gencode; // Pour les expressions
    struct t_range range;
    struct s_array sarray;
    struct {
        stype type;
        union {
            int  ival;
            bool bval;
            char *sval;
        };
    } cte;

    struct {
        stype type;
        struct s_array sarray;
    } ctype;

    struct arglist  *argl;
    struct exprlist *exprl;
}

%token PROGRAM_ IDENT_ NEWLINE_ END_  TWO_POINTS_ ARRAY_ OF_ WRITE_ BEGIN_ READ_ AFFEC_ INT_ BOOL_ STRING_ UNIT_ VAR_ RETURN_ REF_ IF_ THEN_ ELSE_ WHILE_ DO_ DOTCOMMA_ COMMA_ CTE_ PARLEFT_ PARRIGHT_ BRALEFT_ BRARIGHT_ DPOINT_ FUNCTION_ // common tokens
%token MULT_ DIV_ PLUS_ MINUS_ EXP_ INF_ INF_EQ_ SUP_ SUP_EQ_ EQUAL_ DIFF_ AND_ OR_ XOR_ NOT_ // operators (binary or unary)

%type <sval>     IDENT_
%type <cte>      CTE_
%type <gencode>  expr instr program sequence lvalue m fundecllist  fundecl
%type <type>     atomictype
%type <signe>    sign
%type <range>    rangelist
%type <sarray>   arraytype
%type <argl>     identlist varsdecl parlist par

//%type <lstExpr>  exprlist
%type <ctype>    typename
%type <exprl>    exprlist

%left   OR_
%left   AND_
%left   EQUAL_
%left   INF_EQ_ INF_ SUP_EQ_ SUP_ DIFF_
%left   PLUS_ MINUS_
%left   MULT_ DIV_
%right  NEG_ NOT_
%right  AFFEC_
%right  EXP_

%start  program

%%

program: PROGRAM_ IDENT_ vardecllist fundecllist instr  {
        newVarInt(&stable, $2, 0);
        progName = strdup($2);

        quad *q  = qGen(Q_MAIN, NULL, NULL, NULL);
        all_code = concat($4.quad, q);
        all_code = concat(all_code, $5.quad);
    }
    ;

vardecllist : %empty                         { }
            | varsdecl                       { }
            | varsdecl DOTCOMMA_ vardecllist { }
           ;

varsdecl: VAR_ identlist DPOINT_ typename {
             arglistPrint($2);
             switch ($4.type) {
                 case S_INT:
                    fprintf(stdout, "integer\n");
                    break;
                 case S_BOOL:
                    fprintf(stdout, "boolean\n");
                    break;
                 case S_STRING:
                    fprintf(stdout, "string\n");
                    break;
                 case S_ARRAY:
                    fprintf(stdout, "array\n");
                    break;

             }
             arglist *al = $2;

              while (al != NULL) {
                  switch ($4.type) {
                      case S_BOOL:
                          newVarBool(&stable, al->id, false);
                          break;
                      case S_INT:
                          newVarInt(&stable, al->id, 0);
                          break;
                      case S_STRING:
                          newVarStr(&stable, al->id, "\"\"");
                          break;
                      case S_ARRAY:
                      // CREER une nouvelle variable de table
                          newVarArray(&stable, al->id, $4.sarray);
                          //printf("nbim = %d\n", $4.sarray.range->ndim);
                       break;
                    default:
                        ferr("cs.y varsdecl identlist An arg has wrong type");
                  }

                  al = al->next;
              }
          }
          ;

identlist : IDENT_ {
                $$ = arglistNew(strdup($1), NULL);
            }
         | IDENT_ COMMA_ identlist {
                arglist *al = arglistNew(strdup($1), NULL);
                $$ = arglistConcat(al, $3);
            }
         ;

typename : atomictype {
            $$.type = $1;
          }
          | arraytype {
            $$.type = S_ARRAY;
            $$.sarray = $1;
          }
         ;

atomictype : UNIT_   { $$ = S_UNIT;    }
           | BOOL_   { $$ = S_BOOL;    }
           | INT_    { $$ = S_INT;     }
           | STRING_ { $$ = S_STRING;  }
           ;


arraytype : ARRAY_ BRALEFT_ rangelist BRARIGHT_ OF_ atomictype {
              //creer la struct s_array
              printRange(&($3));
              t_range *rg = &($3);
              s_array *arr = malloc(sizeof(s_array));
              if (arr == NULL) {
                  fprintf(stderr, "malloc error\n");
                  exit(EXIT_FAILURE);
              }
              arr->range = rg;
              arr->type = $6;
              // calcul nombre d'elements
              int cpt = 0 ;
              t_range *cur = rg;
              while (cur != NULL) {
                  cpt += (cur->max - cur->min + 1);
                  cur = cur->next;
              }
              arr->size = cpt;
              switch (arr->type) {
                  case S_INT:
                    printf("toto = %d\n", cpt);
                    arr->intarr = newLstInt(arr->size);
                    break;

                  case S_BOOL:
                  arr->boolarr = newLstBool(arr->size);
                    break;
              }
              //fprintf(stdout, "ndim = %d\n", rg->ndim);
              $$ = *arr ;
            }
          ;

rangelist : sign CTE_ TWO_POINTS_ sign CTE_  {
              // test array types
                if ($2.type != S_INT || $5.type != S_INT) {
                    fprintf(stderr, "Mauvais types array \n");
                    exit(EXIT_FAILURE);
                }
                // test borns values
                int min = $2.ival, max = $5.ival;
                if ($1 == '-')
                    min = -min;
                if ($4 == '-')
                    max = -max;
                if (min > max) {
                    fprintf(stderr, "Bornes inf > Borne sup\n");
                    exit(EXIT_FAILURE);
                }
                $$.min = min  ;
                $$.max = max ;
                $$.ndim = 1 ;
                $$.next = NULL;

            }
            |sign CTE_ TWO_POINTS_ sign CTE_ COMMA_ rangelist {
                if ($2.type != S_INT || $5.type != S_INT) {
                    fprintf(stderr, "Mauvais types array \n");
                    exit(EXIT_FAILURE);
                }
                int min = $2.ival, max = $5.ival;
                if ($1 == '-')
                    min = -min;
                if ($4 == '-')
                    max = -max;
                if (min > max) {
                    fprintf(stderr, "Bornes inf > Borne sup\n");
                    exit(EXIT_FAILURE);
                }
                $$.min = min  ;
                $$.max = max ;
                $$.ndim = $7.ndim + 1 ;
                $$.next = &($7) ;


            }
            ;

sign : %empty                       {}
      | MINUS_  {  $$ = '-'; }
      ;

fundecllist : %empty {
                    $$.quad = NULL;
                }
            | fundecl DOTCOMMA_ fundecllist {
                    $$.quad = concat($1.quad, $3.quad);
                }
           ;

fundecl : FUNCTION_ IDENT_ PARLEFT_ parlist PARRIGHT_ DPOINT_ atomictype vardecllist instr {
                arglist *al = $4;
                symbol *fs  = newVarFun(&stable, $2, al, $7);
                quad *qdec  = qGen(Q_FUNDEC, NULL, fs, NULL);
                quad *qend  = qGen(Q_FUNEND, NULL, fs, NULL);

                $$.quad = concat(qdec, $9.quad);
                $$.quad = concat($$.quad, qend);
            }
        ;

parlist : %empty {
                $$ = NULL;
            }
        | par {
                $$ = $1;
            }
        | par COMMA_ par {
                $$ = arglistConcat($1, $3);
            }
        ;

par : IDENT_ DPOINT_ typename {
            symbol *s;
            switch ($3.type) {
                case S_INT  : s = newVarInt(&stable, $1, 0)      ; break ;
                case S_BOOL : s = newVarBool(&stable, $1, false) ; break ;
                default: ferr("cs.y par : IDENT_ DPOINT_ typename Incorrect typename");
            }

            $$ = arglistNew(NULL, s);
        }

    | REF_ IDENT_ DPOINT_ typename {

        }
    ;

instr: lvalue AFFEC_ expr {
                if ($1.ptr->type != $3.ptr->type)
                    ferr("cs.y instr: lvalue and expr type differ");

                quad *q    = qGen(Q_AFFEC, $1.ptr, $3.ptr, NULL);
                quad *quad = concat($3.quad, q);
                $$.quad    = quad;
                $$.ptr     = $1.ptr;
            }
        | RETURN_ expr {}
        | RETURN_ {}
        | IDENT_ PARLEFT_ exprlist PARRIGHT_ {
                funcallExpression(&($$.quad), &($$.ptr), $1, $3);
            }
        | IDENT_ PARLEFT_ PARRIGHT_ {
                funcallExpression(&($$.quad), &($$.ptr), $1, NULL);
            }
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
              symbol *ptr = search(stable, $1);
              testID(ptr, $1);

              // calcul l'indice du exprlist element
              lstInt *tab  = ptr->array.intarr;
              int index    = 0; //  exprlist
              arglist *cur = $3->al;
              int value    = getNthIntVal (tab, $3->al->sym->ival);
              ptr->ival    = value;
              $$.ptr       = ptr;
              $$.quad      = NULL;
            }
      ;

exprlist : expr {
                // TODO
                // TODO
                // TODO
                // TODO
                // TODO
                // TODO
                // TODO
                // TODO
                // TODO
                // TODO
                // TODO
                // TODO
                // GROS PROBLEME PUTIN FCEST QUOI CE MERDIER CEST IMPOSSIBLE LE PREMIER ID EST BON ET LE SECOND EST NULL
                // TODO
                // TODO
                // TODO
                // TODO
                $$->al   = arglistNew(NULL, $1.ptr);
                printf(" ### avant exprlist expr, ID = %s\n\n", $1.ptr->id);
                $$->quad = $1.quad;
                printf(" ### apres exprlist expr, ID = %s\n\n", $1.ptr->id);
            }
        |  expr COMMA_ exprlist {
                arglist *al = arglistNew(NULL, $1.ptr);
                $$->al      = arglistConcat(al, $3->al);
                $$->quad    = concat($1.quad, $3->quad);
            }
        ;

expr :  expr PLUS_ expr {
            if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
              ferr("cs.y expr PLUS expr type error");

            arithmeticExpression(Q_PLUS, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
          }

      | expr MINUS_ expr {
            if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
                ferr("cs.y expr MULT expr type error");

            arithmeticExpression(Q_MINUS, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
          }

      | MINUS_ expr %prec NEG_ {
          if ($2.ptr->type != S_INT)
              ferr("cs.y MINUS expr INT type error");

          symbol *ptr = newTmpInt(&stable, 0);
          symbol *tmp = newTmpInt(&stable, 0);
          $$.ptr      = ptr;
          quad *q     = qGen(Q_MINUS, ptr, tmp, $2.ptr);
          $$.quad     = $2.quad;
          $$.quad     = concat($$.quad, q);
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

      | expr EXP_ expr {
          if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
              ferr("cs.y expr MULT expr type error");

          arithmeticExpression(Q_EXP, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
      }

      | expr OR_ m expr {
            if ($1.ptr->type != $4.ptr->type || $1.ptr->type != S_BOOL)
                ferr("cs.y expr OR expr type error");

            symbol *ptr = newTmpBool(&stable, false);
            $$.ptr      = ptr;


            complete($1.lfalse, false, $3.quad->res);
            $$.lfalse = $4.lfalse;
            $$.ltrue  = concat($1.ltrue, $4.ltrue);

            quad *q   = qGen(Q_OR, ptr, $1.ptr, $4.ptr);
            $$.quad   = concat($1.quad, $3.quad);
            $$.quad   = concat($$.quad, $4.quad);
            $$.quad   = concat($$.quad, q);
          }

      | expr AND_ m expr {
            if ($1.ptr->type != $4.ptr->type || $1.ptr->type != S_BOOL)
                ferr("cs.y expr AND expr type error");
            symbol *ptr = newTmpBool(&stable, false);
            $$.ptr      = ptr;

            complete($1.ltrue, true, $3.quad->res);
            $$.lfalse = concat($1.lfalse, $4.lfalse);
            $$.ltrue  = $4.ltrue;

            quad *q = qGen(Q_AND, ptr, $1.ptr, $4.ptr);
            $$.quad = concat($1.quad, $3.quad);
            $$.quad = concat($$.quad, $4.quad);
            $$.quad = concat($$.quad, q);
          }

      | expr XOR_ m expr {
            if ($1.ptr->type != $4.ptr->type || $1.ptr->type != S_BOOL)
                ferr("cs.y expr XOR expr type error");

            symbol *ptr = newTmpBool(&stable, false);
            $$.ptr      = ptr;

            // complete true false etc

            quad *q = qGen(Q_XOR, ptr, $1.ptr, $4.ptr);
            $$.quad = concat($1.quad, $3.quad);
            $$.quad = concat($$.quad, $4.quad);
            $$.quad = concat($$.quad, q);
          }

       | expr SUP_ expr {

             if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
               ferr("cs.y expr SUP_ expr type error");

             booleanExpression(Q_SUP, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }

       | expr SUP_EQ_ expr {
             if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
               ferr("cs.y expr SUP_ expr type error");

             booleanExpression(Q_SUPEQ, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }
       | expr INF_ expr {
             if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
               ferr("cs.y expr SUP_ expr type error");

             booleanExpression(Q_INF, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }
       | expr INF_EQ_ expr {
             if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
               ferr("cs.y expr SUP_ expr type error");

             booleanExpression(Q_INFEQ, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }
       | expr EQUAL_ expr {
             if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
               ferr("cs.y expr SUP_ expr type error");

             booleanExpression(Q_EQUAL, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }
       | expr DIFF_ expr {
             if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
               ferr("cs.y expr SUP_ expr type error");

             booleanExpression(Q_DIFF, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }

       | NOT_ expr {
           if ($2.ptr->type != S_BOOL)
               ferr("cs.y expr NOT type error");

           symbol *ptr = newTmpBool(&stable, false);
           $$.ptr      = ptr;

           quad *q = qGen(Q_NOT, ptr, $2.ptr, NULL);
           $$.quad = $2.quad;
           $$.quad = concat($$.quad, q);
       }

      | IDENT_ PARLEFT_ exprlist PARRIGHT_ {
                // function call (with parameters)
                funcallExpression(&($$.quad), &($$.ptr), $1, $3);
            }
      | IDENT_ PARLEFT_ PARRIGHT_ {
                funcallExpression(&($$.quad), &($$.ptr), $1, NULL);
            }
      | IDENT_ BRALEFT_ exprlist BRARIGHT_ {

            // expr  = t [2,3]

        arglist *toto = $3->al;
        while (toto != NULL) {
            fprintf(stdout, "liste indices de la ligne 654  %d\n", toto->sym->ival);
            toto = toto->next;
        }
             }
      | IDENT_ {
            symbol *ptr = search(stable, $1);
            testID(ptr, $1);

            $$.ptr    = ptr;
            $$.quad   = NULL;
            $$.ltrue  = NULL;
            $$.lfalse = NULL;
            free($1);
        }
      | CTE_ {
              symbol *ptr;
              switch ($1.type) {
                  case S_INT    : ptr = newTmpInt (&stable, $1.ival);
                                  break;
                  case S_BOOL   : ptr = newTmpBool(&stable, $1.bval);
                                  break;
                  case S_STRING : ptr = newTmpStr (&stable, $1.sval);
                                  break;
                  default: ferr("cs.y expr : CTE_ Unknow cte type");
              }

              $$.ptr    = ptr;
              $$.quad   = NULL;
              $$.ltrue  = NULL;
              $$.lfalse = NULL;
          }
      | PARLEFT_ expr PARRIGHT_ {
              $$.ptr    = $2.ptr;
              $$.quad   = $2.quad;
              $$.ltrue  = $2.ltrue;
              $$.lfalse = $2.lfalse;
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
