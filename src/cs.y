%{
    #include <stdio.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <math.h>
    #include <time.h>
    #include <getopt.h>
    #include <unistd.h>
    #include "../headers/stable.h"
    #include "../headers/quad.h"
    #include "../headers/mips.h"
    #include "../headers/arglist.h"
    #include "../headers/opti.h"
    #include "../headers/array.h"
    #define YYDEBUG 0

    void yyerror  (char *s);
    int yylex     (void);
    void freeLex  (void);
    extern FILE *yyin;
    extern int  linecpt ;
    extern int validComment ;
    symbol *stable = NULL; // global tos
    quad *all_code = NULL; // all quads
    char *progName = NULL;

    // current parsing function symbol or NULL
    static symbol *curfun = NULL;

    symbol ** curtos (void) {
        if (curfun == NULL)
            return &stable;

        return &curfun->fdata->tos;
    }

    arglist *funcalls = NULL;

    void testID (symbol *s, char *name) {
        if(s == NULL) {
            char s[LEN];
            snprintf(s, LEN, "cs.y ID \"%s\" not exists", name);
            fundata *fdata = curfun->fdata;
            stablePrint(fdata->tos);
            ferr(linecpt,s);
        }
    }

    void arithmeticExpression (qop op, symbol **res, quad **quadRes, quad *quad1, symbol *arg1, quad *quad2, symbol *arg2) {
        // OK
        symbol *ptr = newTmpInt(curtos(), 0);
        quad *q     = qGen(op, ptr, arg1, arg2);

        *res        = ptr;
        *quadRes    = concat(quad1, quad2);
        *quadRes    = concat(*quadRes, q);
     }

    void booleanExpression (qop op, symbol **res, quad **quadRes, quad *quad1, symbol *arg1, quad *quad2, symbol *arg2) {
        // OK
        symbol *ptr  = newTmpInt(curtos(), 0);
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
            ferr(linecpt,"cs.y funcallExpression symbol is not a function");

        symbol *res;
        fundata *fdata = fs->fdata;

        switch (fdata->rtype) {
            case S_INT  : res = newTmpInt(curtos(), 0)      ; break ;
            case S_BOOL : res = newTmpBool(curtos(), false) ; break ;
            case S_UNIT : res = NULL                        ; break ;
            default: ferr(linecpt,"cs.y funcallExpression wrong return type");
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
%nonassoc IFX
%nonassoc ELSE_

%start  program

%%

program: PROGRAM_ IDENT_ vardecllist fundecllist instr  {
        symbol *ptr = newProg(&stable, $2);
        progName    = strdup($2);
        $$.ltrue    = NULL;
        $$.lfalse   = NULL;
        $$.quad     = $5.quad;
        $$.ptr      = ptr;
        quad *q     = qGen(Q_MAIN, NULL, NULL, NULL);
        all_code    = concat($4.quad, q);
        all_code    = concat(all_code, $5.quad);
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
                 case S_ARRAY:
                    fprintf(stdout, "array\n");
                    break;
                default: ferr(linecpt,"cs.y varsdecl : VAR_ identlist DPOINT_ typename - wrong typename");

             }
             arglist *al = $2.al;

              while (al != NULL) {
                  switch ($4.type) {
                      case S_BOOL:
                          newVarBool(curtos(), al->id, false, curfun, false);
                          break;
                      case S_INT:
                          newVarInt(curtos(), al->id, 0, curfun, false);
                          break;
                      case S_ARRAY:
                          newVarArray(curtos(), al->id, $4.sarray, curfun, false);
                          break;
                    default:
                        ferr(linecpt,"cs.y varsdecl identlist An arg has wrong type");
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
            $$.type   = S_ARRAY;
            $$.sarray = $1;
          }
         ;

atomictype : UNIT_   { $$ = S_UNIT;    }
           | BOOL_   { $$ = S_BOOL;    }
           | INT_    { $$ = S_INT;     }
           ;


arraytype : ARRAY_ BRALEFT_ rangelist BRARIGHT_ OF_ atomictype {
                s_array *arr = malloc(sizeof(s_array));
                if (arr == NULL)
                    ferr(linecpt,"cs.y arraytype : ARRAY_ BRALEFT_ rangelist BRARIGHT_ OF_ atomictype - malloc");

                 arr->dims  = $3;
                 arr->ndims = $3->dim;
                 arr->type  = $6;
                 arr->index = 1;

                 int size = 1;
                 dimProp *cur = $3;

                 // calculate array total size
                 while (cur != NULL) {
                     size *= (cur->max - cur->min + 1);
                     cur   = cur->next;
                 }

                 arr->size = size;
                if (arr->type != S_INT && arr->type != S_BOOL)
                    ferr(linecpt, "cs.y arraytype : ARRAY_ BRALEFT_ rangelist BRARIGHT_ OF_ atomictype - wrong array type");

                 $$ = arr;
            }
          ;

rangelist : CTE_ TWO_POINTS_ CTE_ {
                if ($1.type != S_INT || $3.type != S_INT)
                    ferr(linecpt,"cs.y rangelist : CTE_ TWO_POINTS_ CTE_ - wrong CTE_ type");

                if ($1.ival > $3.ival)
                    ferr(linecpt,"cs.y rangelist : CTE_ TWO_POINTS_ CTE_ - wrong range");

                $$ = malloc(sizeof(dimProp));
                if ($$ == NULL)
                    ferr(linecpt,"cs.y rangelist : CTE_ TWO_POINTS_ CTE_ - malloc");

                $$->dim  = 1;
                $$->min  = $1.ival;
                $$->max  = $3.ival;
                $$->next = NULL;
            }

        | CTE_ TWO_POINTS_ CTE_ COMMA_ rangelist {
            if ($1.type != S_INT || $3.type != S_INT)
                ferr(linecpt,"cs.y rangelist : CTE_ TWO_POINTS_ CTE_ COMMA_ rangelist - wrong CTE_ type");

            if ($1.ival > $3.ival)
                ferr(linecpt,"cs.y rangelist : CTE_ TWO_POINTS_ CTE_ COMMA_ rangelist - wrong range");

            $$ = malloc(sizeof(dimProp));
            if ($$ == NULL )
              ferr(linecpt,"cs.y rangelist : CTE_ TWO_POINTS_ CTE_ COMMA_ rangelist - malloc");

            $$->dim  = $5->dim + 1;
            $$->min  = $1.ival;
            $$->max  = $3.ival;
            $$->next = $5;
        }
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
                fundata *fdata = curfun->fdata;
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
                case S_INT   : s = newVarInt(curtos(), $1, 0, curfun, false)           ; break ;
                case S_BOOL  : s = newVarBool(curtos(), $1, false, curfun, false)      ; break ;
                case S_ARRAY : s = newVarArray(curtos(), $1, $3.sarray, curfun, false) ; break ;
                default: ferr(linecpt,"cs.y par : IDENT_ DPOINT_ typename Incorrect typename");
            }

            $$.al = arglistNew(NULL, s);
        }

    | REF_ IDENT_ DPOINT_ typename {
            symbol *s;
            switch ($4.type) {
                case S_INT   : s = newVarInt(curtos(), $2, 0, curfun, true)              ; break ;
                case S_BOOL  : s = newVarBool(curtos(), $2, false, curfun, true)         ; break ;
                case S_ARRAY : s = newVarArray(curtos(), $2, $4.sarray, curfun, true)    ; break ;
                default: ferr(linecpt,"cs.y par : REF_ IDENT_ DPOINT_ typename Incorrect typename");
            }

            $$.al = arglistNew(NULL, s);
        }
    ;

instr: lvalue AFFEC_ expr {
                if ($1.ptr->type == S_ARRAY) {
                    if ($3.ptr->type != $1.ptr->arr->type)
                        ferr(linecpt,"cs.y instr: lvalue AFFEC_ expr - lvalue array type != expr type");
                } else if ($3.ptr->type == S_ARRAY) {
                    if ($1.ptr->type != $3.ptr->arr->type)
                        ferr(linecpt,"cs.y instr: lvalue AFFEC_ expr - lvalue type != expr array type");
                } else if ($1.ptr->type != $3.ptr->type)
                    ferr(linecpt,"cs.y instr: lvalue AFFEC_ expr - lvalue type != expr type");

                symbol *sargs = NULL;
                if ($1.ptr->type == S_ARRAY) {
                    sargs = newTmpInt(curtos(), 0);
                    sargs->args = $1.ptr->arr->args;
                } else if ($3.ptr->type == S_ARRAY) {
                    sargs = newTmpInt(curtos(), 0);
                    sargs->args = $3.ptr->arr->args;
                }

                quad *q    = qGen(Q_AFFEC, $1.ptr, $3.ptr, sargs);
                quad *quad = concat($3.quad, q);
                $$.quad    = quad;
                $$.ptr     = $1.ptr;
            }
        | RETURN_ expr {
                if (curfun == NULL)
                    ferr(linecpt,"cs.y instr : RETURN_ expr - Not in function");

                if ($2.ptr->type != curfun->fdata->rtype)
                    ferr(linecpt,"cs.y instr : RETURN_ expr - Return expr type != fun ret type");

                quad *qr = qGen(Q_FUNRETURN, NULL, curfun, $2.ptr);
                $$.quad  = concat($2.quad, qr);
            }
        | RETURN_ {
                if (curfun == NULL)
                    ferr(linecpt,"cs.y instr : RETURN_ - Not in function");

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
        | READ_ lvalue {
                if ($2.ptr->type != S_INT && $2.ptr->type != S_BOOL && $2.ptr->type != S_ARRAY)
                    ferr(linecpt,"cs.y instr : READ_ lvalue - type cannot be read");

                symbol *sargs = NULL;
                if ($2.ptr->type == S_ARRAY) {
                    sargs = newTmpInt(curtos(), 0);
                    sargs->args = $2.ptr->arr->args;
                }

                quad *q = qGen(Q_READ, $2.ptr, sargs, NULL);
                $$.quad = concat($2.quad, q);
            }
        | WRITE_ expr {
                if ($2.ptr->type != S_INT && $2.ptr->type != S_BOOL && $2.ptr->type != S_STRING && $2.ptr->type != S_ARRAY)
                    ferr(linecpt,"cs.y instr : WRITE_ expr - type cannot be write");

                symbol *s = NULL;
                if ($2.ptr->type == S_ARRAY)
                    s = newTmpInt(curtos(), $2.ptr->arr->index);

                quad *q = qGen(Q_WRITE, NULL, $2.ptr, s);
                $$.quad = concat($2.quad, q);

            }
        | IF_ expr THEN_ instr m %prec IFX {
                quad *qif   = qGen(Q_IF, NULL, $2.ptr, NULL);
                qif->gtrue  = NULL;
                qif->gfalse = $5.quad->res;
                qif->gnext  = $5.quad->res;

                $$.quad = concat($2.quad, qif);
                $$.quad = concat($$.quad, $4.quad);
                $$.quad = concat($$.quad, $5.quad);
            }
        | IF_ expr THEN_ instr ELSE_ m instr m {
                quad *qif   = qGen(Q_IF, NULL, $2.ptr, NULL);
                qif->gtrue  = NULL;
                qif->gfalse = $6.quad->res;
                qif->gnext  = $8.quad->res;

                quad *go = qGen(Q_GOTO, qif->gnext, NULL, NULL);

                $$.quad = concat($2.quad, qif);
                $$.quad = concat($$.quad, $4.quad);
                $$.quad = concat($$.quad, go);
                $$.quad = concat($$.quad, $6.quad);
                $$.quad = concat($$.quad, $7.quad);
                $$.quad = concat($$.quad, $8.quad);
            }
        | WHILE_ m expr DO_ instr m {
                quad *qif   = qGen(Q_IF, NULL, $3.ptr, NULL);
                qif->gtrue  = NULL;
                qif->gfalse = $6.quad->res;
                qif->gnext  = $2.quad->res;

                quad *go = qGen(Q_GOTO, qif->gnext, NULL, NULL);

                $$.quad = concat($2.quad, $3.quad);
                $$.quad = concat($$.quad, qif);
                $$.quad = concat($$.quad, $5.quad);
                $$.quad = concat($$.quad, go);
                $$.quad = concat($$.quad, $6.quad);
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
                if (ptr->type != S_ARRAY)
                    ferr(__LINE__, "lvalue : IDENT_ BRALEFT_ exprlist BRARIGHT_ - type != S_ARRAY");

                ptr->arr->args = $3.al;
                $$.ptr  = ptr;
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
              ferr(linecpt,"cs.y expr PLUS expr type error");

            arithmeticExpression(Q_PLUS, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
          }

      | expr MINUS_ expr {
            if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
                ferr(linecpt,"cs.y expr MULT expr type error");

            arithmeticExpression(Q_MINUS, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
          }

      | MINUS_ expr %prec NEG_ {
          if ($2.ptr->type != S_INT)
              ferr(linecpt,"cs.y MINUS expr INT type error");

          symbol *ptr = newTmpInt(curtos(), 0);
          symbol *tmp = newTmpInt(curtos(), 0);
          $$.ptr      = ptr;
          quad *q     = qGen(Q_MINUS, ptr, tmp, $2.ptr);
          $$.quad     = $2.quad;
          $$.quad     = concat($$.quad, q);
      }

       | expr MULT_ expr {
           if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
               ferr(linecpt,"cs.y expr MULT expr type error");
            arithmeticExpression(Q_MULT, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
          }

      | expr DIV_ expr {
            if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
                ferr(linecpt,"cs.y expr MULT expr type error");
            if ($3.ptr->ival == 0)
                ferr(linecpt,"Error division by 0");
            arithmeticExpression(Q_DIV, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
          }

      | expr MOD_ expr {
          if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
              ferr(linecpt,"cs.y expr MOD expr type error");

          arithmeticExpression(Q_MOD, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
      }

      | expr EXP_ expr {
          if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
              ferr(linecpt,"cs.y expr MULT expr type error");

          arithmeticExpression(Q_EXP, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
      }

      | expr OR_ m expr {
            if ($1.ptr->type != $4.ptr->type || $1.ptr->type != S_BOOL)
                ferr(linecpt,"cs.y expr OR expr type error");

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
                ferr(linecpt,"cs.y expr AND expr type error");
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
                ferr(linecpt,"cs.y expr XOR expr type error");

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
               ferr(linecpt,"cs.y expr SUP_ expr type error");

             booleanExpression(Q_SUP, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }

       | expr SUP_EQ_ expr {
             if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
               ferr(linecpt,"cs.y expr SUP_ expr type error");

             booleanExpression(Q_SUPEQ, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }
       | expr INF_ expr {
             if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
               ferr(linecpt,"cs.y expr SUP_ expr type error");

             booleanExpression(Q_INF, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }
       | expr INF_EQ_ expr {
             if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
               ferr(linecpt,"cs.y expr SUP_ expr type error");

             booleanExpression(Q_INFEQ, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }
       | expr EQUAL_ expr {
             if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
               ferr(linecpt,"cs.y expr SUP_ expr type error");

             booleanExpression(Q_EQUAL, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }
       | expr DIFF_ expr {
             if ($1.ptr->type != $3.ptr->type || $1.ptr->type != S_INT)
               ferr(linecpt,"cs.y expr SUP_ expr type error");

             booleanExpression(Q_DIFF, &($$.ptr), &($$.quad), $1.quad, $1.ptr, $3.quad, $3.ptr);
           }

       | NOT_ expr {
           if ($2.ptr->type != S_BOOL)
               ferr(linecpt,"cs.y expr NOT type error");

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
          symbol *ptr = search(stable, curfun, $1);
          testID(ptr, $1);

          if (ptr->type != S_ARRAY)
              ferr(__LINE__, "expr : IDENT_ BRALEFT_ exprlist BRARIGHT_ - type != S_ARRAY");

          ptr->arr->args = $3.al;
          symbol *arrVal;

          switch (ptr->arr->type) {
              case S_INT  : arrVal = newTmpInt(curtos(), 0)      ; break;
              case S_BOOL : arrVal = newTmpBool(curtos(), false) ; break;
              default: ferr(linecpt, "cs.y expr : IDENT_ BRALEFT_ exprlist BRARIGHT_ - array wrong type");
          }

          symbol *sargs = newTmpInt(curtos(), 0);
          sargs->args = $3.al;
          quad *q = qGen(Q_AFFEC, arrVal, ptr, sargs);
          $$.ptr  = arrVal;
          $$.quad = q;
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
                  default: ferr(linecpt,"cs.y expr : CTE_ Unknow cte type");
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

void yyerror (char *s) {
    ferr(linecpt, s);
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
                if (res < 0 || res >= LEN)
                    ferr(__LINE__, "cs.y main snprintf");

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

    #if YYDEBUG == 1
          yydebug = 1;
    #endif
    // Je sais pas pourquoi les options move l'indice du nom scalpa selon le nombres d'options ptdr
    yyin = fopen(argv[opt], "r");
    if (yyin == NULL)
        ferr(__LINE__, "cs.y main - error fopen");

    yyparse();
    if(!validComment)
        ferr(linecpt,"error comments \n") ;
    char out[LEN];

    res = snprintf(out, LEN, "%s.s", o ? filename : (*progName ? progName : "out"));
    if (res < 0 || res >= LEN)
        ferr(__LINE__, "cs.y main - snprintf");

    FILE *output = fopen(out, "w");
    #if YYDEBUG == 1
        qPrint(all_code);
    #endif

    optiLoop(&all_code, &stable);
    getMips(output, stable, all_code);

    if (tos)
        stablePrint(stable);

    freeLex();
    qFree(all_code);
    sFree(stable);

    return EXIT_SUCCESS;
}
