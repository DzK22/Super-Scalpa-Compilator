%{
    #include <stdio.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <math.h>
    #include <getopt.h>
    #include <unistd.h>
    #include "../headers/stable.h"
    #include "../headers/quad.h"
    #include "../headers/mips.h"
    #include "../headers/arglist.h"
    #define YYDEBUG 1

    int yyerror  (char *s);
    int yylex    (void);
    void freeLex (void);
    extern FILE *yyin;

    symbol *stable = NULL; // global tos
    quad *all_code = NULL; // all quads
    char *progName = NULL;

    // current parsing function symbol or NULL
    static symbol *curfun = NULL;

    void printRange (dimProp *list) {
        dimProp *cur = list;
        while (cur != NULL) {
                fprintf(stdout, "min:%d[]max:%d[]ndim:%d\n", cur->min, cur->max, cur->dim);
            cur = cur->next;
        }
    }


    symbol ** curtos () {
        if (curfun == NULL)
            return &stable;
        return &((fundata *) curfun->fdata)->tos;
    }

    arglist *funcalls = NULL;

    void testID (symbol *s, char *name) {
        if(s == NULL) {
            char s[LEN];
            snprintf(s, LEN, "cs.y ID \"%s\" not exists", name);
            fundata *fdata = (fundata *) curfun->fdata;
            stablePrint(fdata->tos);
            ferr(s);
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

    // qd et al = NULL for procedure (no args)
    void funcallExpression (quad **quadRes, symbol **symRes, char *id, quad *qd, arglist *al) {
        symbol *fs = search(stable, curfun, id);
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
        if (al)
            slist = arglistToSymlist(al);

        quad *q  = qGen(Q_FUNCALL, res, fs, slist);
        *quadRes = NULL;

        if (qd)
            *quadRes = concat(*quadRes, qd);

        *quadRes = concat(*quadRes, q);
        *symRes  = res;
     }
%}

%union {
    int   ival;
    bool  bval;
    char  *sval;
    char  signe;
    stype type;
    qop   op;
    struct arr_range *dimprop;
    struct s_array *sarray;

    struct {
        struct symbol *ptr;
        struct quad   *quad;
        struct quad   *ltrue;
        struct quad   *lfalse;
    } gencode; // Pour les expressions
    struct {
        stype type;
        union {
            int  ival;
            bool bval;
            char *sval;
        };
    } cte;

    struct {
        struct arglist *al;
        char   *id;
        symbol *sym; // arg symbol (only for function args)
    } argl;

    struct {
        struct quad *quad;
        struct arglist *al;
    } exprl;

    struct {
        stype type;
        struct s_array *sarray;
    } ctype;
}

%token PROGRAM_ IDENT_ NEWLINE_ END_  TWO_POINTS_ ARRAY_ OF_ WRITE_ BEGIN_ READ_ AFFEC_ INT_ BOOL_ STRING_ UNIT_ VAR_ RETURN_ REF_ IF_ THEN_ ELSE_ WHILE_ DO_ DOTCOMMA_ COMMA_ CTE_ PARLEFT_ PARRIGHT_ BRALEFT_ BRARIGHT_ DPOINT_ FUNCTION_ // common tokens
%token MULT_ DIV_ PLUS_ MINUS_ EXP_ INF_ INF_EQ_ SUP_ SUP_EQ_ EQUAL_ DIFF_ AND_ OR_ XOR_ NOT_ MOD_// operators (binary or unary)

%type <sval>     IDENT_
%type <cte>      CTE_
%type <gencode>  expr instr program sequence lvalue m fundecllist  fundecl
%type <type>     atomictype
%type <signe>    sign
%type <argl>     identlist varsdecl parlist par
%type <exprl>    exprlist
%type <sarray>    arraytype
%type <dimprop>    rangelist
%type <ctype>    typename

%left   OR_
%left   AND_
%left   EQUAL_
%left   INF_EQ_ INF_ SUP_EQ_ SUP_ DIFF_
%left   PLUS_ MINUS_
%left   MULT_ DIV_ MOD_
%right  NEG_ NOT_
%right  AFFEC_
%right  EXP_

%start  program

%%

program: PROGRAM_ IDENT_ vardecllist fundecllist instr  {
        symbol *ptr = newProg(&stable, $2);
        progName = strdup($2);
        $$.ltrue = NULL;
        $$.lfalse = NULL;
        $$.quad = $5.quad;
        $$.ptr = ptr;
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
             arglistPrint($2.al);
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
             arglist *al = $2.al;

              while (al != NULL) {
                  switch ($4.type) {
                      case S_BOOL:
                          newVarBool(&stable, al->id, false, curfun);
                          break;
                      case S_INT:
                          newVarInt(&stable, al->id, 0, curfun);
                          break;
                      case S_STRING:
                          newVarStr(&stable, al->id, "\"\"", curfun);
                          break;
                      case S_ARRAY:
                      // CREER une nouvelle variable de table
                          newVarArray(&stable, al->id, $4.sarray);
                          break;
                    default:
                        ferr("cs.y varsdecl identlist An arg has wrong type");
                  }

                  al = al->next;
              }
          }
          ;

identlist : IDENT_ {
                $$.al = arglistNew(strdup($1), NULL);
            }
         | IDENT_ COMMA_ identlist {
                arglist *al = arglistNew(strdup($1), NULL);
                $$.al = arglistConcat(al, $3.al);
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
                s_array *arr = malloc(sizeof(s_array));
                if (arr == NULL) {
                    fprintf(stderr, "malloc error\n");
                     exit(EXIT_FAILURE);
                 }
                 //printRange(rg);
                 arr->dims = $3;
                 arr->type = $6;
                 arr->index = 1;
                 int cpt = 1;
                 dimProp *cur = $3;
                 //calcule de size
                 while (cur != NULL) {
                     cpt *= (cur->max - cur->min + 1);
                     cur = cur->next;
                 }
                 arr->size = cpt;
                 switch (arr->type) {
                     case S_INT:
                        arr->values = newLstInt(arr->size);
                        break;
                     case S_BOOL:
                        break;
                 }
                 /*lstInt *toto = arr->values;
                 while (toto != NULL) {
                     fprintf(stdout, "%d\n", toto->ival);
                     toto = toto->next;
                 }*/
                 $$ = arr;
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

                $$ = malloc(sizeof(dimProp));
                if($$ == NULL )
                {
                  fprintf(stderr,"error malloc  \n") ;
                  exit(0)  ;
                }
                $$->dim = 1;
                $$->min = min;
                $$->max = max;
                $$->next = NULL;
                fprintf(stdout, "borne inf : %d et borne sup : %d et dims : %d\n", $$->min, $$->max, $$->dim);

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
                $$ = malloc(sizeof(dimProp));
                if($$ == NULL )
                {
                  fprintf(stderr,"error malloc  \n") ;
                  exit(0)  ;
                }
                $$->dim = $7->dim + 1;
                $$->min = min;
                $$->max = max;
                $$->next = $7;
                fprintf(stdout, "borne inf : %d et borne sup : %d et dims : %d\n", $$->min, $$->max, $$->dim);
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

fundecl : FUNCTION_ IDENT_ PARLEFT_ {
                // we should init the function symbol as soon as possible to have curfun set
                symbol *fs  = newVarFun(&stable, $2);
                curfun      = fs;
            }
          parlist PARRIGHT_ DPOINT_ atomictype  vardecllist {
                fundata *fdata = (fundata *) curfun->fdata;
                fdata->al      = $5.al;
                fdata->rtype   = $8;
            }
          instr {
                quad *qdec = qGen(Q_FUNDEC, NULL, curfun, NULL);
                quad *qend = qGen(Q_FUNEND, NULL, curfun, NULL);

                curfun  = NULL;
                $$.quad = concat(qdec, $11.quad);
                $$.quad = concat($$.quad, qend);
            }
        ;

parlist : %empty {
                $$.al = NULL;
            }
        | par {
                $$ = $1;
            }
        | par COMMA_ parlist {
                $$.al = arglistConcat($1.al, $3.al);
            }
        ;

par : IDENT_ DPOINT_ typename {
            symbol *s;
            switch ($3.type) {
                case S_INT  : s = newVarInt(curtos(), $1, 0, curfun)      ; break ;
                case S_BOOL : s = newVarBool(curtos(), $1, false, curfun) ; break ;
                case S_ARRAY : break;
                default: ferr("cs.y par : IDENT_ DPOINT_ typename Incorrect typename");
            }

            $$.al = arglistNew(NULL, s);
        }

    | REF_ IDENT_ DPOINT_ typename {

        }
    ;

instr: lvalue AFFEC_ expr {
                if ($1.ptr->type == S_ARRAY) {
                  if($3.ptr->type != $1.ptr->arr->type) {
                      printf("type lvalue %d \n ", $1.ptr->arr->type) ;
                      printf("type expr %d \n ", $3.ptr->type) ;
                      ferr("cs.y instr: lvalue AFFEC_ expr - 1");
                  }
                }
                else if ($3.ptr->type == S_ARRAY ) {
                  if($1.ptr->type != $3.ptr->arr->type) {
                      printf("type lvalue %d \n ", $1.ptr->type) ;
                      printf("type expr %d \n ", $3.ptr->type) ;
                      ferr("cs.y instr: lvalue AFFEC_ expr - 2");
                  }
                }
                else if ($1.ptr->type != $3.ptr->type) {
                  printf("type lvalue %d \n ", $1.ptr->type) ;
                  printf("type expr %d \n ", $3.ptr->type) ;
                  ferr("cs.y instr: lvalue AFFEC_ expr - 3");
                }
                if ($1.ptr->type == S_ARRAY)
                    printf("TATALAND = %d\n", $1.ptr->arr->index);
                quad *q    = qGen(Q_AFFEC, $1.ptr, $3.ptr, NULL);
                quad *quad = concat($3.quad, q); // segfault here for array affectation
                $$.quad    = quad;
                $$.ptr     = $1.ptr;

            }
        | RETURN_ expr {
                if (curfun == NULL)
                    ferr("cs.y instr : RETURN_ expr - Not in function");

                if ($2.ptr->type != ((fundata *) curfun->fdata)->rtype)
                    ferr("cs.y instr : RETURN_ expr - Return expr type != fun ret type");

                quad *qr = qGen(Q_FUNRETURN, NULL, curfun, $2.ptr);
                $$.quad  = concat($2.quad, qr);
            }
        | RETURN_ {
                if (curfun == NULL)
                    ferr("cs.y instr : RETURN_ - Not in function");

                $$.quad = qGen(Q_FUNRETURN, NULL, curfun, NULL);
            }
        | IDENT_ PARLEFT_ exprlist PARRIGHT_ {
                funcallExpression(&($$.quad), &($$.ptr), $1, $3.quad, $3.al);
            }
        | IDENT_ PARLEFT_ PARRIGHT_ {
                funcallExpression(&($$.quad), &($$.ptr), $1, NULL, NULL);
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
                symbol *ptr = search(stable, curfun, $1);
                testID(ptr, $1);

                $$.ptr  = ptr;
                $$.quad = NULL;
                free($1);
            }

        | IDENT_ BRALEFT_ exprlist BRARIGHT_ {
              symbol *ptr = search(stable, curfun, $1);
               testID(ptr, $1);
              // calcul de la valeur de l'indice du tableau
              arglist *indicesLst = $3.al;
              dimProp *dimension = ptr->arr->dims;
             // printf("YOOO\n");
              int cpt = 1, curind, dimnum = 1;;
              while (dimension != NULL && indicesLst != NULL) {
                  //printf("dims min %d || dims max %d\n",dimension->min, dimension->max);
                  curind = indicesLst->sym->ival;
                  if (curind < dimension->min || curind > dimension->max) {
                      fprintf(stderr, "Indice %d out of bound on dim n°%d\n", curind, dimnum);
                      exit(EXIT_FAILURE);
                  }
                  cpt *= indicesLst->sym->ival;
                  dimension = dimension->next;
                  indicesLst = indicesLst->next;
                  dimnum++;
              }
              ptr->arr->index = cpt;

              //printf("TOTOLAND: %d\n", ptr->arr->index);
              //printf("SIZE = %d\n", ptr->arr->size);
               if (ptr->arr->index > ptr->arr->size) {
                  fprintf(stderr, "index out of range\n");
                  exit(EXIT_FAILURE);
              }
              $$.ptr = ptr;
              $$.quad = NULL ;
            }
      ;

exprlist : expr {
                $$.al   = arglistNew(NULL, $1.ptr);
                $$.quad = $1.quad;
             }
        |  expr COMMA_ exprlist {
                arglist *al = arglistNew(NULL, $1.ptr);
                $$.al       = arglistConcat(al, $3.al);
                $$.quad     = concat($1.quad, $3.quad);
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

          symbol *ptr = newTmpInt(curtos(), 0);
          symbol *tmp = newTmpInt(curtos(), 0);
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
            if ($3.ptr->ival == 0)
                ferr("Error division by 0");
            arithmeticExpression(Q_DIV, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
          }

      | expr MOD_ expr {
          if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
              ferr("cs.y expr MOD expr type error");

          arithmeticExpression(Q_MOD, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
      }

      | expr EXP_ expr {
          if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
              ferr("cs.y expr MULT expr type error");

          arithmeticExpression(Q_EXP, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
      }

      | expr OR_ m expr {
            if ($1.ptr->type != $4.ptr->type || $1.ptr->type != S_BOOL)
                ferr("cs.y expr OR expr type error");

            symbol *ptr = newTmpBool(curtos(), false);
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
            symbol *ptr = newTmpBool(curtos(), false);
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

            symbol *ptr = newTmpBool(curtos(), false);
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

           symbol *ptr = newTmpBool(curtos(), false);
           $$.ptr      = ptr;

           quad *q = qGen(Q_NOT, ptr, $2.ptr, NULL);
           $$.quad = $2.quad;
           $$.quad = concat($$.quad, q);
       }

      | IDENT_ PARLEFT_ exprlist PARRIGHT_ {
                // function call (with parameters)
                funcallExpression(&($$.quad), &($$.ptr), $1, $3.quad, $3.al);
            }
      | IDENT_ PARLEFT_ PARRIGHT_ {
                funcallExpression(&($$.quad), &($$.ptr), $1, NULL, NULL);
            }
      | IDENT_ BRALEFT_ exprlist BRARIGHT_ {

      /*  symbol *ptr = search(stable, curfun, $1);
        testID(ptr, $1);

        // mettre le type de retour
        $$.ptr->type = ptr->arr->type ;
        $$.quad = NULL ;

        // calcul de l'indice dans le tableau
        int index = 0 ;
        arglist *toto = $3.al;
             while (toto != NULL) {
            fprintf(stdout, "liste indices de expr = ident[i, .. ,n]  %d\n", toto->sym->ival);
             toto = toto->next;
        }*/

             }
      | IDENT_ {
            symbol *ptr = search(stable, curfun, $1);
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
                  case S_INT    :
                                  ptr = newTmpInt (curtos(), $1.ival);
                                  break;
                  case S_BOOL   : ptr = newTmpBool(curtos(), $1.bval);
                                  break;
                  case S_STRING : ptr = newTmpStr (curtos(), $1.sval);
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
          $$.quad = qGen(Q_LABEL, newTmpLabel(curtos()), NULL, NULL);
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
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <scalpa_file> [-v --version] [-t --tos] [-o] <progName>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char filename[LEN];
    char arg;
    bool tos = false, version = false, o = false;
    int opt = 1, res;
    while (1) {
        static struct option long_options[] =
        {
          {"version",     no_argument,   0, 'v'},
          {"tos", no_argument, 0, 't'},
          {"o", required_argument, 0, 'o'},
          {0, 0, 0, 0}
        };
        int option_index = 0;
        arg = getopt_long_only(argc, argv, "", long_options, &option_index);
        if (arg == -1)
            break;
        switch (arg) {
            case 'v':
                version = true;
                opt++;
                break;
            case 't':
                tos = true;
                opt++;
                break;
            case 'o':
                o = true;
                res = snprintf(filename, LEN, "%s", optarg);
                printf("tata : %s\n", filename);
                if (res < 0 || res >= LEN) {
                    fprintf(stderr, "snprintf error\n");
                    return EXIT_FAILURE;
                }
                opt += 2;
                break;
            default:
                fprintf(stderr, "Usage: %s <scalpa_file>\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (version) {
        fprintf(stdout, "Members:\n");
        fprintf(stdout, "Danyl El-kabir\nFrançois Grabenstaetter\nJérémy Bach\nNadjib Belaribi\n");
    }

    #if YYDEBUG
          yydebug = 1;
    #endif
    // Je sais pas pourquoi les options move l'indice du nom scalpa selon le nombres d'options ptdr
    yyin = fopen(argv[opt], "r");
    if (yyin == NULL) {
        fprintf(stderr, "Error fopen\n");
        return EXIT_FAILURE;
    }
    yyparse();
    char out[LEN];

    res = snprintf(out, LEN, "%s.s", o ? filename : (*progName ? progName : "out"));
    if (res < 0 || res >= LEN) {
        fprintf(stderr, "snprintf error\n");
        return EXIT_FAILURE;
    }

    FILE *output = fopen(out, "w");
    if (yydebug)
        qPrint(all_code);
    getMips(output, stable, all_code);
    if (tos)
        stablePrint(stable);
    freeLex();
    qFree(all_code);
    sFree(stable);

    return EXIT_SUCCESS;
}
