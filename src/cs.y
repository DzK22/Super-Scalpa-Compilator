%{
    #include <time.h>
    #include <getopt.h>
    #include "../headers/stable.h"
    #include "../headers/quad.h"
    #include "../headers/mips.h"
    #include "../headers/list.h"
    #include "../headers/opti.h"
    #include "../headers/array.h"
    #include "../headers/util.h"
    #define YYDEBUG 0

    static char tbuf[LEN], tbuf2[LEN];
    void yyerror  (char *s);
    int yylex     (void);
    void freeLex  (void);

    extern FILE *yyin;
    extern int   linecpt ;
    extern int   validComment ;

    symbol *stable = NULL; // global tos
    quad *all_code = NULL; // all quads
    char *progName = NULL;
    char *colors[4] = { PURPLE, CYAN, PINK, GREEN };

    #define yferr(str) { \
        snpt(snprintf(tbuf2, LEN, "%s (source line %d)", str, linecpt)); \
        ferr(tbuf2); }

    // current parsing function symbol or NULL
    static symbol *curfun = NULL;

    symbol ** curtos (void) {
        if (curfun == NULL)
            return &stable;

        return &curfun->fdata->tos;
    }

    list *funcalls = NULL;

    void testID (symbol *s, char *name) {
        if(s == NULL) {
            stablePrintAll(stable);
            snpt(snprintf(tbuf, LEN, "ID \"%s\" not exists", name));
            yferr(tbuf);
        }
    }

    void arithmeticExpression (qop op, symbol **res, quad **quadRes, quad *quad1, symbol *arg1, quad *quad2, symbol *arg2) {
        // OK
        symbol *ptr = newTmpInt(curtos(), 0);


        quad *q     = qGen(op, ptr, arg1, arg2);

        *res        = ptr;
        *quadRes    = qConcat(quad1, quad2);
        *quadRes    = qConcat(*quadRes, q);
     }

    void booleanExpression (qop op, symbol **res, quad **quadRes, quad *quad1, symbol *arg1, quad *quad2, symbol *arg2) {
        // OK
        symbol *ptr  = newTmpBool(curtos(), 0);

        quad *q      = qGen(op, ptr, arg1, arg2);

        *res         = ptr;
        *quadRes     = qConcat(quad1, quad2);
        *quadRes     = qConcat(*quadRes, q);
        (*res)->type = S_BOOL;
     }

    // qd et al = NULL for procedure (no args)
    void funcallExpression (quad **quadRes, symbol **symRes, char *id, quad *qd, list *al) {
        symbol *fs = search(stable, curfun, id);
        testID(fs, id);
        free(id);

        if (fs->type != S_FUNCTION) {
            snpt(snprintf(tbuf, LEN, "funcallExpression symbol is not a function"));
            ferr(tbuf);
        }

        symbol *res;
        fundata *fdata = fs->fdata;

        switch (fdata->rtype) {
            case S_INT  : res = newTmpInt(curtos(), 0)  ; break ;
            case S_BOOL : res = newTmpBool(curtos(), 0) ; break ;
            case S_UNIT : res = NULL                    ; break ;
            default: yferr("funcallExpression wrong return type");
        }

        quad *q  = qGen(Q_FUNCALL, res, fs, listToSymlist(al));
        *quadRes = NULL;
        listFree(al);

        if (qd)
            *quadRes = qConcat(*quadRes, qd);

        *quadRes = qConcat(*quadRes, q);
        *symRes  = res;
     }

    void transIfArray (quad **qar, symbol **sar, list *args) {
        if ((*sar)->type != S_ARRAY || args == NULL)
            return;

        symbol *arrVal;
        switch ((*sar)->arr->type) {
            case S_INT  : arrVal = newTmpInt(curtos(), 0)  ; break;
            case S_BOOL : arrVal = newTmpBool(curtos(), 0) ; break;
            default: yferr("transIfArray - Array wrong type");
        }

        quad *q = qGen(Q_AFFEC, arrVal, *sar, listToSymlist(args));
        *qar    = qConcat(*qar, q);
        *sar    = arrVal;
        listFree(args);
    }
%}

%union {
    int   ival;
    bool  bval;
    char  *sval;
    stype type;
    qop   op;
    struct arr_range *dimprop;
    struct s_array *sarray;

    struct {
        struct symbol *ptr;
        struct quad   *quad;
        struct list   *args;
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
        struct list *al;
        char   *id;
        symbol *sym; // arg symbol (only for function args)
    } argl;

    struct {
        struct quad *quad;
        struct list *al;
    } exprl;

    struct {
        stype type;
        struct s_array *sarray;
    } ctype;
}

%token PROGRAM_ NEWLINE_ END_ THEN_ TWO_POINTS_ ARRAY_ OF_ WRITE_ BEGIN_ READ_ AFFEC_ INT_ BOOL_ STRING_ UNIT_ VAR_ RETURN_ REF_ IF_ ELSE_ WHILE_ DO_ DOTCOMMA_ COMMA_ PARLEFT_ PARRIGHT_ BRALEFT_ BRARIGHT_ DPOINT_ FUNCTION_ // common tokens
%token MULT_ DIV_ PLUS_ MINUS_ EXP_ INF_ INF_EQ_ SUP_ SUP_EQ_ EQUAL_ DIFF_ AND_ OR_ XOR_ NOT_ MOD_// operators (binary or unary)

%token <sval>    IDENT_
%token <cte>     CTE_
%type <gencode>  expr instr program sequence lvalue m fundecllist  fundecl
%type <type>     atomictype
%type <argl>     identlist varsdecl parlist par
%type <exprl>    exprlist
%type <sarray>   arraytype
%type <dimprop>  rangelist
%type <ctype>    typename

%left   OR_ XOR_
%left   AND_
%left   EQUAL_ DIFF_
%left   INF_EQ_ INF_ SUP_EQ_ SUP_
%left   PLUS_ MINUS_
%left   MULT_ DIV_ MOD_
%right  EXP_
%precedence  NEG_ NOT_

%start  program

%%

program: PROGRAM_ IDENT_ vardecllist fundecllist instr  {
        symbol *ptr = newProg(&stable, $2);
        progName    = strdup($2);
        free($2);

        $$.quad = $5.quad;
        $$.ptr  = ptr;
        $$.args = NULL;

        quad *q  = qGen(Q_MAIN, NULL, NULL, NULL);
        all_code = qConcat($4.quad, q);
        all_code = qConcat(all_code, $5.quad);
    }
    ;

vardecllist : %empty                         { }
            | varsdecl                       { }
            | varsdecl DOTCOMMA_ vardecllist { }
           ;

varsdecl: VAR_ identlist DPOINT_ typename {
             listPrint($2.al);

             switch ($4.type) {
                 case S_INT:
                    printf("integer\n");
                    break;
                 case S_BOOL:
                    printf("boolean\n");
                    break;
                 case S_ARRAY:
                    printf("array\n");
                    break;
                default: yferr("varsdecl : VAR_ identlist DPOINT_ typename - wrong typename");

             }
             list *al = $2.al;

              while (al != NULL) {
                  switch ($4.type) {
                      case S_BOOL:
                          newVarBool(curtos(), al->id, 0, curfun, false);
                          break;
                      case S_INT:
                          newVarInt(curtos(), al->id, 0, curfun, false);
                          break;
                      case S_ARRAY:
                          newVarArray(curtos(), al->id, $4.sarray, curfun, false);
                          break;
                    default:
                        yferr("varsdecl identlist An arg has wrong type");
                  }

                  al = al->next;
              }
             listFree($2.al);
          }
          ;

identlist : IDENT_ {
                $$.al = listNew(strdup($1), NULL);
                free($1);
            }
         | IDENT_ COMMA_ identlist {
                list *al = listNew(strdup($1), NULL);
                $$.al = listConcat(al, $3.al);
                free($1);
            }
         ;

typename : atomictype {
            $$.type = $1;
          }
          | arraytype {
            $$.type   = S_ARRAY;
            $$.sarray = $1;
          }
         ;

atomictype : UNIT_   { $$ = S_UNIT;    }
           | BOOL_   { $$ = S_BOOL;    }
           | INT_    { $$ = S_INT;     }
           ;

arraytype : ARRAY_ BRALEFT_ rangelist BRARIGHT_ OF_ atomictype {
                 //Si le type de valeurs des tableaux n'est ni bool ni int alors erreur
                 if ($6 != S_INT && $6 != S_BOOL)
                    yferr("arraytype : ARRAY_ BRALEFT_ rangelist BRARIGHT_ OF_ atomictype - wrong array type");

                 $$ = initArray($3, $6);
            }
          ;

rangelist : CTE_ TWO_POINTS_ CTE_ {
                if ($1.type != S_INT || $3.type != S_INT)
                    yferr("rangelist : CTE_ TWO_POINTS_ CTE_ - wrong CTE_ type");

                if ($1.ival > $3.ival)
                    yferr("rangelist : CTE_ TWO_POINTS_ CTE_ - wrong range");

                $$ = initDimProp($1.ival, $3.ival, NULL);
            }

        | CTE_ TWO_POINTS_ CTE_ COMMA_ rangelist {
            if ($1.type != S_INT || $3.type != S_INT)
                yferr("rangelist : CTE_ TWO_POINTS_ CTE_ COMMA_ rangelist - wrong CTE_ type");

            if ($1.ival > $3.ival)
                yferr("rangelist : CTE_ TWO_POINTS_ CTE_ COMMA_ rangelist - wrong range");

            $$ = initDimProp($1.ival, $3.ival, $5);
        }
        ;

fundecllist : %empty {
                    $$.quad = NULL;
                }
            | fundecl DOTCOMMA_ fundecllist {
                    $$.quad = qConcat($1.quad, $3.quad);
                }
           ;

fundecl : FUNCTION_ IDENT_ PARLEFT_ {
                // we should init the function symbol as soon as possible to have curfun set
                symbol *fs  = newVarFun(&stable, $2);
                curfun      = fs;
                free($2);
            }
          parlist PARRIGHT_ DPOINT_ atomictype  vardecllist {
                fundata *fdata = curfun->fdata;
                fdata->al      = $5.al;
                fdata->rtype   = $8;
            }
          instr {
                quad *qdec = qGen(Q_FUNDEC, NULL, curfun, NULL);
                quad *qend = qGen(Q_FUNEND, NULL, curfun, NULL);

                curfun  = NULL;
                $$.quad = qConcat(qdec, $11.quad);
                $$.quad = qConcat($$.quad, qend);
            }
        ;

parlist : %empty {
                $$.al = NULL;
            }
        | par {
                $$ = $1;
            }
        | par COMMA_ parlist {
                $$.al = listConcat($1.al, $3.al);
            }
        | UNIT_ { $$.al = NULL; }
        ;

par : IDENT_ DPOINT_ typename {
            symbol *s;

            switch ($3.type) {
                case S_INT   : s = newVarInt(curtos(), $1, 0, curfun, false)           ; break ;
                case S_BOOL  : s = newVarBool(curtos(), $1, 0, curfun, false)      ; break ;
                case S_ARRAY : s = newVarArray(curtos(), $1, $3.sarray, curfun, false) ; break ;
                default: yferr("par : IDENT_ DPOINT_ typename Incorrect typename");
            }
            free($1);
            $$.al = listNew(NULL, s);
        }

    | REF_ IDENT_ DPOINT_ typename {
            symbol *s;
            switch ($4.type) {
                case S_INT   : s = newVarInt(curtos(), $2, 0, curfun, true)              ; break ;
                case S_BOOL  : s = newVarBool(curtos(), $2, 0, curfun, true)             ; break ;
                case S_ARRAY : s = newVarArray(curtos(), $2, $4.sarray, curfun, true)    ; break ;
                default: yferr("par : REF_ IDENT_ DPOINT_ typename Incorrect typename");
            }

            free($2);
            $$.al = listNew(NULL, s);
        }
    ;

instr: lvalue AFFEC_ expr {
                transIfArray(&($3.quad), &($3.ptr), $3.args);



                if ($1.ptr->type == S_ARRAY) {
                    if ($3.ptr->type != $1.ptr->arr->type)
                        yferr("instr: lvalue AFFEC_ expr - lvalue array type != expr type");
                } else if ($1.ptr->type != $3.ptr->type)
                    yferr("instr: lvalue AFFEC_ expr - lvalue type != expr type");

                if ($1.ptr->type == S_ARRAY)
                    $$.args = $1.args;
                else
                    $$.args = NULL;

                quad *q = qGen(Q_AFFEC, $1.ptr, $3.ptr, listToSymlist($$.args));
                $$.quad = qConcat($1.quad, $3.quad);
                $$.quad = qConcat($$.quad, q);
                $$.ptr  = $1.ptr;
                listFree($$.args);
            }
        | RETURN_ expr {
                if (curfun == NULL)
                    yferr("instr : RETURN_ expr - Not in function");


                transIfArray(&($2.quad), &($2.ptr), $2.args);

                if ($2.ptr->type != curfun->fdata->rtype)
                    yferr("instr : RETURN_ expr - Return expr type != fun ret type");

                quad *qr = qGen(Q_FUNRETURN, NULL, curfun, $2.ptr);
                $$.quad  = qConcat($2.quad, qr);
                $$.args  = NULL;
            }
        | RETURN_ {
                if (curfun == NULL)
                    yferr("instr : RETURN_ - Not in function");

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
                $$.args = NULL;
            }
        | BEGIN_ END_ { $$.ptr = NULL; $$.quad = NULL; $$.args = NULL;}
        | READ_ lvalue {
                if ($2.ptr->type != S_INT && $2.ptr->type != S_BOOL && $2.ptr->type != S_ARRAY)
                    yferr("instr : READ_ lvalue - Type cannot be read");



                if ($2.ptr->type == S_ARRAY)
                    $$.args = $2.args;
                else
                    $$.args = NULL;

                quad *q = qGen(Q_READ, $2.ptr, listToSymlist($$.args), NULL);
                $$.quad = qConcat($2.quad, q);
                listFree($$.args);
            }
        | WRITE_ expr {
                transIfArray(&($2.quad), &($2.ptr), $2.args);


                if ($2.ptr->type != S_INT && $2.ptr->type != S_BOOL && $2.ptr->type != S_STRING)
                    yferr("instr : WRITE_ expr - Type cannot be write");

                quad *q = qGen(Q_WRITE, NULL, $2.ptr, NULL);
                $$.quad = qConcat($2.quad, q);
                $$.args = NULL;

            }
        | IF_ expr THEN_ instr m {
                transIfArray(&($2.quad), &($2.ptr), $2.args);


                if ($2.ptr->type != S_BOOL)
                    yferr("instr : IF_ expr THEN_ instre m - We need bool expr here");

                quad *qif   = qGen(Q_IF, NULL, $2.ptr, NULL);
                qif->gfalse = $5.quad->res;

                $$.quad = qConcat($2.quad, qif);
                $$.quad = qConcat($$.quad, $4.quad);
                $$.quad = qConcat($$.quad, $5.quad);

                $$.ptr  = NULL;
                $$.args = NULL;
            }
        | IF_ expr THEN_ instr ELSE_ m instr m {
                transIfArray(&($2.quad), &($2.ptr), $2.args);


                if ($2.ptr->type != S_BOOL)
                    yferr("instr : IF_ expr THEN_ instre m - We need bool expr here");

                quad *qif   = qGen(Q_IF, NULL, $2.ptr, NULL);
                qif->gfalse = $6.quad->res;
                quad *gnext = qGen(Q_GOTO, $8.quad->res, NULL, NULL);

                $$.quad = qConcat($2.quad, qif);
                $$.quad = qConcat($$.quad, $4.quad);
                $$.quad = qConcat($$.quad, gnext);
                $$.quad = qConcat($$.quad, $6.quad);
                $$.quad = qConcat($$.quad, $7.quad);
                $$.quad = qConcat($$.quad, $8.quad);

                $$.ptr  = NULL;
                $$.args = NULL;
            }
        | WHILE_ m expr DO_ instr m {
                transIfArray(&($3.quad), &($3.ptr), $3.args);


                if ($3.ptr->type != S_BOOL)
                    yferr("instr : IF_ expr THEN_ instre m - We need bool expr here");

                quad *qif   = qGen(Q_IF, NULL, $3.ptr, NULL);
                qif->gfalse = $6.quad->res;
                quad *gnext = qGen(Q_GOTO, $2.quad->res, NULL, NULL);

                $$.quad = qConcat($2.quad, $3.quad);
                $$.quad = qConcat($$.quad, qif);
                $$.quad = qConcat($$.quad, $5.quad);
                $$.quad = qConcat($$.quad, gnext);
                $$.quad = qConcat($$.quad, $6.quad);

                $$.ptr  = NULL;
                $$.args = NULL;
            }
      ;

sequence : instr DOTCOMMA_ sequence  {
                $$.quad = qConcat($1.quad, $3.quad);
                $$.ptr  = NULL;
                $$.args = NULL;
             }
             | instr DOTCOMMA_ {
                 $$.quad = $1.quad;
                 $$.ptr  = NULL;
                 $$.args = NULL;
             }
             | instr  {
                 $$.quad = $1.quad;
                 $$.ptr  = NULL;
                 $$.args = NULL;
             }
        ;

lvalue: IDENT_ {
                symbol *ptr = search(stable, curfun, $1);
                testID(ptr, $1);

                $$.ptr  = ptr;
                $$.quad = NULL;
                $$.args = NULL;
                free($1);
            }

        | IDENT_ BRALEFT_ exprlist BRARIGHT_ {
                symbol *ptr = search(stable, curfun, $1);
                testID(ptr, $1);

                if (ptr->type != S_ARRAY)
                    yferr("lvalue : IDENT_ BRALEFT_ exprlist BRARIGHT_ - Type != S_ARRAY");

                free($1);

                if (!testArrayIndices(ptr->arr->dims, $3.al))
                    yferr("lvalue : IDENT_ BRALEFT_ exprlist BRARIGHT_ - Indices doesnt match");

                $$.ptr  = ptr;
                $$.quad = $3.quad;
                $$.args = $3.al;
            }
      ;

exprlist : expr {
                transIfArray(&($1.quad), &($1.ptr), $1.args);

                $$.al   = listNew(NULL, $1.ptr);
                $$.quad = $1.quad;
             }
        |  expr COMMA_ exprlist {
                transIfArray(&($1.quad), &($1.ptr), $1.args);

                list *al = listNew(NULL, $1.ptr);
                $$.al    = listConcat(al, $3.al);
                $$.quad  = qConcat($1.quad, $3.quad);
            }
        ;

expr :  expr PLUS_ expr {
            transIfArray(&($1.quad), &($1.ptr), $1.args);
            transIfArray(&($3.quad), &($3.ptr), $3.args);




            if ($1.ptr->type != S_INT || $1.ptr->type != $3.ptr->type)
              yferr("expr: expr PLUS expr - Type error");

            arithmeticExpression(Q_PLUS, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
          }

      | expr MINUS_ expr {
          transIfArray(&($1.quad), &($1.ptr), $1.args);
          transIfArray(&($3.quad), &($3.ptr), $3.args);




            if ($1.ptr->type != S_INT || $1.ptr->type != $3.ptr->type)
                yferr("expr : expr MINUS_ expr - Type error");

            arithmeticExpression(Q_MINUS, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
          }

      | MINUS_ expr %prec NEG_ {
          transIfArray(&($2.quad), &($2.ptr), $2.args);



           if ($2.ptr->type != S_INT)
              yferr("expr : MINUS expr INT - Type error");

          symbol *ptr = newTmpInt(curtos(), 0);
          symbol *tmp = newTmpInt(curtos(), 0);
          $$.ptr      = ptr;

          quad *q = qGen(Q_MINUS, ptr, tmp, $2.ptr);
          $$.quad = $2.quad;
          $$.quad = qConcat($$.quad, q);
          $$.args = NULL;
      }

       | expr MULT_ expr {
           transIfArray(&($1.quad), &($1.ptr), $1.args);
           transIfArray(&($3.quad), &($3.ptr), $3.args);

           if ($1.ptr->type != S_INT || $1.ptr->type != $3.ptr->type)
               yferr("expr : expr MULT expr - Type error");





            arithmeticExpression(Q_MULT, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
          }

      | expr DIV_ expr {
          transIfArray(&($1.quad), &($1.ptr), $1.args);
          transIfArray(&($3.quad), &($3.ptr), $3.args);




            if ($1.ptr->type != S_INT || $1.ptr->type != $3.ptr->type)
                yferr("expr : expr DIV expr - Type error");
            if ($3.ptr->ival == 0)
                yferr("expr : expr DIV expr - Division by 0");

            arithmeticExpression(Q_DIV, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
          }

      | expr MOD_ expr {
          transIfArray(&($1.quad), &($1.ptr), $1.args);
          transIfArray(&($3.quad), &($3.ptr), $3.args);

          if ($1.ptr->type != S_INT || $1.ptr->type != $3.ptr->type)
              yferr("expr : expr MOD expr - Type error");




          arithmeticExpression(Q_MOD, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
      }

      | expr EXP_ expr {
          transIfArray(&($1.quad), &($1.ptr), $1.args);
          transIfArray(&($3.quad), &($3.ptr), $3.args);

          if ($1.ptr->type != S_INT || $1.ptr->type != $3.ptr->type)
              yferr("expr : expr EXP expr - Type error");





          arithmeticExpression(Q_EXP, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
      }

      | expr OR_ expr {
          transIfArray(&($1.quad), &($1.ptr), $1.args);
          transIfArray(&($3.quad), &($3.ptr), $3.args);

            if ($1.ptr->type != S_BOOL || $1.ptr->type != $3.ptr->type)
                yferr("expr : expr OR expr - Type error");

            symbol *ptr = newTmpBool(curtos(), 0);
            $$.ptr      = ptr;

            quad *q   = qGen(Q_OR, ptr, $1.ptr, $3.ptr);
            $$.quad   = qConcat($1.quad, $3.quad);
            $$.quad   = qConcat($$.quad, q);




            $$.args = NULL;
          }

      | expr AND_ expr {
          transIfArray(&($1.quad), &($1.ptr), $1.args);
          transIfArray(&($3.quad), &($3.ptr), $3.args);

            if ($1.ptr->type != S_BOOL || $1.ptr->type != $3.ptr->type)
                yferr("expr : expr AND expr - Type error");

            symbol *ptr = newTmpBool(curtos(), 0);

            $$.ptr      = ptr;

            quad *q = qGen(Q_AND, ptr, $1.ptr, $3.ptr);
            $$.quad = qConcat($1.quad, $3.quad);
            $$.quad = qConcat($$.quad, q);





            $$.args = NULL;
          }

      | expr XOR_ expr {
          transIfArray(&($1.quad), &($1.ptr), $1.args);
          transIfArray(&($3.quad), &($3.ptr), $3.args);

            if ($1.ptr->type != S_BOOL || $1.ptr->type != $3.ptr->type)
                yferr("expr : expr XOR expr - Type error");

            symbol *ptr = newTmpBool(curtos(), 0);

             $$.ptr      = ptr;

            quad *q = qGen(Q_XOR, ptr, $1.ptr, $3.ptr);
            $$.quad = qConcat($1.quad, $3.quad);
            $$.quad = qConcat($$.quad, q);



            $$.args = NULL;
          }

       | expr SUP_ expr {
           transIfArray(&($1.quad), &($1.ptr), $1.args);
           transIfArray(&($3.quad), &($3.ptr), $3.args);

             if ($1.ptr->type != S_INT || $1.ptr->type != $3.ptr->type)
               yferr("expr : expr SUP_ expr - Type error");





             booleanExpression(Q_SUP, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }

       | expr SUP_EQ_ expr {
           transIfArray(&($1.quad), &($1.ptr), $1.args);
           transIfArray(&($3.quad), &($3.ptr), $3.args);

             if ($1.ptr->type != S_INT || $1.ptr->type != $3.ptr->type)
               yferr("expr : expr SUP_EQ expr - Type error");




             booleanExpression(Q_SUPEQ, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }
       | expr INF_ expr {
           transIfArray(&($1.quad), &($1.ptr), $1.args);
           transIfArray(&($3.quad), &($3.ptr), $3.args);

             if ($1.ptr->type != S_INT || $1.ptr->type != $3.ptr->type)
               yferr("expr : expr INF_ expr - Type error");




             booleanExpression(Q_INF, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }
       | expr INF_EQ_ expr {
           transIfArray(&($1.quad), &($1.ptr), $1.args);
           transIfArray(&($3.quad), &($3.ptr), $3.args);

             if ($1.ptr->type != S_INT || $1.ptr->type != $3.ptr->type)
               yferr("expr : expr INF_EQ expr - Type error");




             booleanExpression(Q_INFEQ, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }
       | expr EQUAL_ expr {
           transIfArray(&($1.quad), &($1.ptr), $1.args);
           transIfArray(&($3.quad), &($3.ptr), $3.args);

             if ($1.ptr->type != S_INT || $1.ptr->type != $3.ptr->type)
               yferr("expr : expr EQUAL expr - Type error");




             booleanExpression(Q_EQUAL, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }
       | expr DIFF_ expr {
           transIfArray(&($1.quad), &($1.ptr), $1.args);
           transIfArray(&($3.quad), &($3.ptr), $3.args);

             if ($1.ptr->type != S_INT || $1.ptr->type != $3.ptr->type)
               yferr("expr : expr DIFF expr - Type error");




             booleanExpression(Q_DIFF, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }

       | NOT_ expr {
           transIfArray(&($2.quad), &($2.ptr), $2.args);

           if ($2.ptr->type != S_BOOL)
               yferr("expr : expr NOT - Type error");

           symbol *ptr = newTmpBool(curtos(), 0);
           $$.ptr      = ptr;



           quad *q = qGen(Q_NOT, ptr, $2.ptr, NULL);
           $$.quad = $2.quad;
           $$.quad = qConcat($$.quad, q);
           $$.args = NULL;
       }

      | IDENT_ PARLEFT_ exprlist PARRIGHT_ {
            // function call (with parameters)
            funcallExpression(&($$.quad), &($$.ptr), $1, $3.quad, $3.al);
        }
      | IDENT_ PARLEFT_ PARRIGHT_ {
            funcallExpression(&($$.quad), &($$.ptr), $1, NULL, NULL);
        }
      | IDENT_ BRALEFT_ exprlist BRARIGHT_ {
          symbol *ptr = search(stable, curfun, $1);
          testID(ptr, $1);
          free($1);

          if (ptr->type != S_ARRAY)
              yferr("expr : IDENT_ BRALEFT_ exprlist BRARIGHT_ - Type != S_ARRAY");

          if (!testArrayIndices(ptr->arr->dims, $3.al))
              yferr("lvalue : IDENT_ BRALEFT_ exprlist BRARIGHT_ - Indices doesnt match");

          $$.ptr  = ptr;
          $$.quad = $3.quad;
          $$.args = $3.al;
         }
      | IDENT_ {
            symbol *ptr = search(stable, curfun, $1);
            testID(ptr, $1);
            free($1);

            $$.ptr  = ptr;
            $$.quad = NULL;
            $$.args = NULL;
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
                                  free($1.sval);
                                  break;
                  default: yferr("expr : CTE_ - Unknow cte type");
              }
              ptr->is_cst = true;
              $$.ptr  = ptr;
              $$.quad = NULL;
              $$.args = NULL;
          }
      | PARLEFT_ expr PARRIGHT_ {
              transIfArray(&($2.quad), &($2.ptr), $2.args);

              $$.ptr  = $2.ptr;
              $$.quad = $2.quad;
              $$.args = NULL;
          }
      ;

m : %empty {
          $$.quad = qGen(Q_LABEL, newTmpLabel(curtos()), NULL, NULL);
      }
  ;

%%

void yyerror (char *s) {
    yferr(s);
}

int yywrap (void) {
    return 1;
}

int main (int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <scalpa_file> [-v --version] [-t --tos] [-o] <progName>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char filename[LEN];
    char arg;
    bool tos = false, version = false, o = false, opti = false;
    int opt = 1, res;

    while (1) {
        static struct option long_options[] =
        {
          {"version",     no_argument,   0, 'v'},
          {"tos", no_argument, 0, 't'},
          {"o", required_argument, 0, 'o'},
          {"opti", no_argument, 0, 'p'},
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
                snpt(snprintf(filename, LEN, "%s", optarg));
                opt += 2;
                break;
            case 'p':
                opti = true;
                opt++;
                break;
            default:
                fprintf(stderr, "Usage: %s <scalpa_file>\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (version) {
        fprintf(stdout, YELLOW"Members:\n"COL_RESET);
        srand(time(NULL));
        arrShuffle(colors, 4);
        fprintf(stdout, "%s", colors[0]);
        fprintf(stdout, "Danyl El-kabir\n"COL_RESET);
        fprintf(stdout, "%s", colors[1]);
        fprintf(stdout, "François Grabenstaetter\n"COL_RESET);
        fprintf(stdout, "%s", colors[2]);
        fprintf(stdout, "Jérémy Bach\n"COL_RESET);
        fprintf(stdout, "%s", colors[3]);
        fprintf(stdout, "Nadjib Belaribi\n"COL_RESET);
        exit(EXIT_FAILURE);
    }

    #if YYDEBUG == 1
          yydebug = 1;
    #endif
    // Je sais pas pourquoi les options move l'indice du nom scalpa selon le nombres d'options ptdr
    yyin = fopen(argv[opt], "r");
    if (yyin == NULL)
        ferr("fopen");

    yyparse();
    if(!validComment)
        yferr("error comments") ;
    char out[LEN];

    snpt(snprintf(out, LEN, "%s.s", o ? filename : (*progName ? progName : "out")));

    FILE *output = fopen(out, "w");
    #if YYDEBUG == 1
        qPrint(all_code);
    #endif

    if (opti)
        optiLoop(&all_code, &stable);
    getMips(output, stable, all_code);

    if (tos)
        stablePrintAll(stable);

    freeLex();
    qFree(all_code);
    if (*progName)
        free(progName);
        
    sFree(stable);

    if (fclose(output) == EOF)
        yferr("fclose error");
    if (fclose(yyin) == EOF)
        yferr("fclose error");

    return EXIT_SUCCESS;
}
