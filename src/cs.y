%{
    #include <stdio.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <math.h>
    #include "../headers/stable.h"
    #include "../headers/quad.h"
    #include "../headers/quad_list.h"
    #include "../headers/mips.h"
    #include "../headers/list.h"
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
%}

%union {
    int  val;
    bool bol;
    char *str;
    stype type;
    qop   op;

    struct {
        struct symbol    *res;
        struct quad      *code;
        struct quad_list *true_list;
        struct quad_list *false_list;
    } gencode; // Pour les expressions

    struct {
        stype type;
        union {
            int  ival;
            char *sval;
            bool bval;
        };
    } cte;

    struct listIdents *listID;
    struct listDecls *listDecl;

}

%token PROGRAM_ IDENT_ NEWLINE_ END_ WRITE_ BEGIN_ READ_ AFFEC_ INT_ BOOL_ UNIT_ VAR_ RETURN_ REF_ IF_ THEN_ ELSE_ DOTCOMMA_ COMMA_ CTE_ PARLEFT_ PARRIGHT_ BRALEFT_ BRARIGHT_ // common tokens
%token MULT_ DIV_ PLUS_ MINUS_ EXP_ INF_ INF_EQ_ SUP_ SUP_EQ_ EQUAL_ DIFF_ AND_ OR_ XOR_ NOT_ // operators (binary or unary)

%type <str>      IDENT_
%type <cte>      CTE_
%type <gencode>  expr instr program sequence lvalue
%type <listDecl> fundecllist  vardecllist fundecl
%type <listID>   identlist varsdecl
%type <type>     typename atomictype
%type <op>       opu opb

%left   PLUS_ MINUS_
%left   MULT_ DIV_
%right  EXP_
%left   NOT_

%start  program
%%

program: PROGRAM_ IDENT_ vardecllist fundecllist instr  {
        newVarInt(&stable, $2, 0);
        progName = strdup($2);
        $$.code  = concat(NULL, $5.code);
        all_code = $$.code;
    }
    ;

vardecllist : %empty                       { }
            | varsdecl
                {
                    $$ = newList($1);
                    addList($$, $1);
                }
            | varsdecl DOTCOMMA_ vardecllist
                {
                    addList($3, $1);
                    $$ = $3;
                }
           ;

varsdecl: VAR_ identlist ':' typename {
            /* Creer une entree dans la table des symboles avec
             le type des variables dans identlist  */
             $$->type = $4 ;
             printID($2);
             listIdents *cur = $2;
             symbol *res;

              while (cur != NULL) {
                  switch ($4) {
                      case S_BOOL:
                          res = newVarBool(&stable, cur->tid, false);
                          break;
                      case S_INT:
                          res = newVarInt(&stable, cur->tid, 0);
                          break;

                  }
                  cur = cur->next;
              }
          }
          ;

identlist : IDENT_ {
                $$->tid  = strdup($1);
                $$->type = S_NONE;
                $$->next = NULL;
            }
         | IDENT_ COMMA_ identlist {
                $$->tid  = strdup($1);
                $$->next = $3;
            }
         ;

typename : atomictype {
            $$ = $1;
          }
         ;

atomictype : UNIT_ { $$ = S_UNIT; }
           | BOOL_ { $$ = S_BOOL; }
           | INT_  { $$ = S_INT;  }
           ;

fundecllist : %empty                        {  }
            | fundecl DOTCOMMA_ fundecllist       {  }
           ;

/* BOUMBOUM = pour ne pas avoir de conflit bison, a virer quand implementation des fonctions mdr */
fundecl : "BOUMBOUM"                            {}
        ;

instr: lvalue AFFEC_ expr {
                quad *q    = qGen(Q_AFFEC, $1.res, $3.res, NULL);
                quad *code = concat($3.code, q);
                $$.code    = code;
                $$.res     = $1.res;
            }
        | RETURN_ expr {}
        | RETURN_ {}
        | IDENT_ PARLEFT_ exprlist PARRIGHT_ {}
        | IDENT_ PARLEFT_ PARRIGHT_ {}
        | BEGIN_ sequence END_ {
                $$.res  = $2.res;
                $$.code = $2.code;
            }
        | BEGIN_ END_ {}
        | READ_ expr {
                $$.res  = $2.res;
                quad *q = qGen(Q_READ, $$.res, NULL, NULL);
                $$.code = concat($2.code, q);
            }
        | WRITE_ expr {
                $$.res  = $2.res;
                quad *q = qGen(Q_WRITE, $$.res, NULL, NULL);
                $$.code = concat($2.code, q);
            }
        | IF_ expr THEN_ instr {
            symbol *goto_true = newLabel(&stable, "");
            symbol *goto_false = newLabel(&stable, "");
            quad *true_label = qGen(Q_LABEL, goto_true, NULL, NULL);
            quad *false_label = qGen(Q_LABEL, goto_false, NULL, NULL);
            completeQuadList($2.true_list, goto_true);
            completeQuadList($2.false_list, goto_false);
            $$.code = concat($2.code, true_label);
            $$.code = concat($$.code, $4.code);
            $$.code = concat($$.code, false_label);
        }
        | IF_ expr THEN_ instr ELSE_ instr {
            symbol *goto_true = newLabel(&stable, "");
            symbol *goto_false = newLabel(&stable, "");
            symbol *goto_next = newLabel(&stable, "");
            completeQuadList($2.true_list, goto_true);
            completeQuadList($2.false_list, goto_false);
            quad *true_label = qGen(Q_LABEL, goto_true, NULL, NULL);
            quad *false_label = qGen(Q_LABEL, goto_false, NULL, NULL);
            quad *next_label = qGen(Q_LABEL, goto_next, NULL, NULL);
            quad *g_next = qGen(Q_GOTO, goto_next, NULL, NULL);
            $$.code = concat($2.code, true_label); // goto true
            $$.code = concat($$.code, $4.code); // code if true
            $$.code = concat($$.code, g_next); // skip else
            $$.code = concat($$.code, false_label); // goto else
            $$.code = concat($$.code, $6.code); // else code
            $$.code = concat($$.code, next_label); // skip the label
        }
      ;

sequence : instr DOTCOMMA_ sequence  {
            $$.code = concat($1.code, $3.code);
            $$.res  = $1.res;
         }
         | instr DOTCOMMA_ {
             $$.res  = $1.res;
             $$.code = $1.code;
         }
         | instr  {
             $$.res  = $1.res;
             $$.code = $1.code;
         }
        ;

lvalue: IDENT_ {
                symbol *res = search(stable, $1);
                testID(res, $1);
                $$.res  = res;
                $$.code = NULL;
                free($1);
            }

        | IDENT_ BRALEFT_ exprlist BRARIGHT_ {

            }
      ;

exprlist : expr {}
        |  expr COMMA_ exprlist {}
        ;

expr : CTE_ {
                symbol *res;
                switch ($1.type) {
                    case S_INT    : res = newTmpInt (&stable, $1.ival);
                                    break;
                    case S_BOOL   : res = newTmpBool(&stable, $1.bval);
                                    break;
                    case S_STRING : res = newTmpStr (&stable, $1.sval);
                                    free($1.sval);
                                    break;
                    default: ferr("cs.y expr : CTE_ Unknow cte type");
                }

                $$.res  = res;
                $$.code = NULL;
            }
      | PARLEFT_ expr PARRIGHT_ {
                $$.res  = $2.res;
                $$.code = $2.code;
                $$.true_list = $2.true_list;
                $$.false_list = $2.false_list;
            }
      | expr opb expr {
                if ($1.res->type != $3.res->type)
                    ferr("cs.y expr : expr opb expr Different types");

                // verify type and opb correct
                stype type = $1.res->type;
                qop op = $2;

                if ((type == S_INT && !(op == Q_PLUS || op == Q_MINUS || op == Q_MULT || op == Q_DIV || op == Q_EXP || op == Q_INF || op == Q_INFEQ || op == Q_SUP || op == Q_SUPEQ || op == Q_EQUAL || op == Q_DIFF))
                || ((type == S_BOOL && !(op == Q_EQUAL || op == Q_DIFF || op == Q_AND || op == Q_OR || op == Q_XOR))
                || (type == S_STRING)))
                    ferr("cs.y expr opb expr Incorrect type/opb");

                // OK
                symbol *res;
                switch (type) {
                    case S_INT  : res = newTmpInt (&stable, 0)     ; break;
                    case S_BOOL : res = newTmpBool(&stable, false) ; break;
                    // no operations on S_STRING are allowed
                }
                quad *q_true, *q_false, *q;
                switch (op) {
                    case Q_MINUS:
                    case Q_PLUS:
                    case Q_MULT:
                    case Q_DIV:
                    case Q_EXP:
                        $$.res = res;
                        q = qGen(op, res, $1.res, $3.res);
                        $$.code = $1.code;
                        $$.code = concat($$.code, $3.code);
                        $$.code = concat($$.code, q);
                        break;
                    case Q_EQUAL:
                    case Q_DIFF:
                    case Q_SUP:
                    case Q_SUPEQ:
                    case Q_INF:
                    case Q_INFEQ:
                        q_true = qGen(op, NULL, $1.res, $3.res);
                        q_false = qGen(Q_GOTO, NULL, NULL, NULL);
                        $$.code = NULL;
                        $$.code = concat($$.code, $1.code);
                        $$.code = concat($$.code, $3.code);
                        $$.code = concat($$.code, q_true);
                        $$.code = concat($$.code, q_false);
                        $$.true_list = newQuadList(q_true);
                        $$.false_list = newQuadList(q_false);
                        break;
                }
            }
      | opu expr {
                // verify type and opb correct
                stype type = $2.res->type;
                qop op = $1;

                if ((type == S_INT && op != Q_MINUS) || (type == S_BOOL && op != Q_NOT) || (type == S_STRING))
                    ferr("cs.y opu expr Incorrect type/opu");

                symbol *res;
                switch (type) {
                    case S_INT  : res = newTmpInt (&stable, 0)     ; break;
                    case S_BOOL : res = newTmpBool(&stable, false) ; break;
                    // no operations on S_STRING are allowed
                }
                $$.res = res;

                quad *q;
                if (op == Q_MINUS)
                    q = qGen(op, res, res, $2.res);
                else
                    q = qGen(op, res, $2.res, NULL);

                $$.code = $2.code;
                $$.code = concat($$.code, q);
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
                symbol *res = search(stable, $1);
                testID(res, $1);
                $$.res  = res;
                $$.code = NULL;
                free($1);
            }
      ;

opb : PLUS_   { $$ = Q_PLUS   ; }
    | MINUS_  { $$ = Q_MINUS  ; }
    | MULT_   { $$ = Q_MULT   ; }
    | DIV_    { $$ = Q_DIV    ; }
    | EXP_    { $$ = Q_EXP    ; }
    | INF_    { $$ = Q_INF    ; }
    | INF_EQ_ { $$ = Q_INFEQ  ; }
    | SUP_    { $$ = Q_SUP    ; }
    | SUP_EQ_ { $$ = Q_SUPEQ  ; }
    | EQUAL_  { $$ = Q_EQUAL  ; }
    | DIFF_   { $$ = Q_DIFF   ; }
    | AND_    { $$ = Q_AND    ; }
    | OR_     { $$ = Q_OR     ; }
    | XOR_    { $$ = Q_XOR    ; }
    ;

opu : MINUS_  { $$ = Q_MINUS ; }
    | NOT_    { $$ = Q_NOT   ; }
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
        // yydebug = 1;
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
