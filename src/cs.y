%{
    #include <stdio.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <math.h>
    #include "../headers/stable.h"
    #include "../headers/quad.h"
    #include "../headers/mips.h"
    #include "../headers/list.h"
    #define YYDEBUG 1
    int yyerror (char *s);
    int yylex (void);
    void freeLex (void);
    extern FILE *yyin;

    symbol *stable = NULL;
    quad *all_code = NULL;
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
    struct {
        struct symbol   *res;
        struct quad     *code;
        struct quad_list *true_list;
        struct quad_list *false_list;
    } gencode; // Pour les expressions

    struct listIdents *listID;
    struct listDecls *listDecl;

}

%token PROGRAM_ IDENT_ NEWLINE_ END_ WRITE_  MULT_ DIV_ PLUS_ MINUS_ EXP_ INF_ INF_EQ_ BEGIN_ READ_
%token SUP_ SUP_EQ_ EQUAL_ DIFF_ AFFEC_ AND_ OR_ XOR_ NOT_ INT_ BOOL_ UNIT_ VAR_ RETURN_ REF_ DOTCOMMA_ COMMA_ BOOLEAN_ INTEGER_ STRING_

%type   <str> IDENT_ STRING_
%type   <val> INTEGER_
%type   <bol> BOOLEAN_
%type   <gencode>  expr instr program sequence //lvalue
%type   <listDecl> fundecllist  vardecllist fundecl
%type   <listID> identlist varsdecl
%type   <type> typename atomictype



%left   PLUS_ MINUS_
%left   MULT_ DIV_
%right  EXP_
%left   NOT_

%start  program
%%

program: PROGRAM_ IDENT_ vardecllist fundecllist instr  {
        newVarInt(&stable, $2, 0);
        $$.code = NULL;
        $$.code = concat($$.code, $5.code);
        all_code = $$.code;
    }
    ;

vardecllist : %empty                       { }
           | varsdecl                      {
             $$ = newList($1);
             addList($$, $1);
           }
           | varsdecl DOTCOMMA_ vardecllist      {
               addList($3, $1);
               $$ = $3;
            }
           ;

varsdecl: VAR_ identlist ':' typename {
        /* Creer une entree dans la table des symboles avec
         le type des variables dans identlist  */
         printID($2);
         $$->type = $4 ;

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

identlist : IDENT_  {
            $$->tid = strdup($1);
            $$->type = S_NONE ;
            $$->next = NULL;
        }
         | IDENT_ COMMA_ identlist  {
            $$->tid = strdup($1);
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
/* REGLE BIDON , A SUPPRIMER APRES mdr */
fundecl : %empty                            {}
        ;

instr: IDENT_ AFFEC_ expr   {
            symbol *res = search(stable, $1);
            testID(stable, $1);
            $$.res = res;
            quad *q  = qGen(Q_AFFEC, res, $3.res, NULL);
            quad *code = concat($3.code, q);
            $$.code = code;
            free($1);
        }
      | RETURN_ expr {}
      | BEGIN_ sequence END_ {
          $$.res = $2.res;
          $$.code = $2.code;
      }
      | BEGIN_ END_ {}
      | WRITE_ expr {
          $$.res = $2.res;
          quad *q = qGen(Q_WRITE, $$.res, NULL, NULL);
          $$.code = concat($2.code, q);
      }
      ;

sequence : instr DOTCOMMA_ sequence  {
            $$.code = concat($1.code, $3.code);
            $$.res = $1.res;
         }
         | instr DOTCOMMA_ {
             $$.res = $1.res;
             $$.code = $1.code;
         }
         | instr  {
             $$.res = $1.res;
             $$.code = $1.code;
         }
        ;
/*
lvalue: IDENT_ {
            symbol *res = search(stable, $1);
            testID(stable, $1);
            $$.res = res;
            $$.code = NULL;
            free($1);
        }
      ;*/

exprlist : expr {}
        |  expr COMMA_ exprlist {}
        ;

expr : INTEGER_                                 {
        symbol *res = newTmpInt(&stable, 0);
        res->type = S_INT;
        res->val = $1;
        //printf("yoo = %d\n", res->val);
        $$.res = res;
        $$.code = NULL;
      }

      | '(' expr ')'                          { }
      | expr PLUS_ expr                         {
           symbol *res = newTmpInt(&stable, 0);
           $$.res = res;
           quad *q = qGen(Q_PLUS, res, $1.res, $3.res);
           quad *code = concat($1.code, $3.code);
           code = concat(code, q);
           $$.code = code;

      }
      | opu expr                              { }
      | IDENT_                                {
          symbol *res = search(stable, $1);
          testID(stable, $1);
          $$.res = res;
          $$.code = NULL;
          free($1);
      }
      | IDENT_ '(' exprlist ')'                { }
      ;

opu : MINUS_                                   {}
    | NOT_                                     {}
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
    FILE *output = fopen("out.s", "w");
    qPrint(all_code);
    getMips(output, stable, all_code);
    freeLex();
    qFree(all_code);
    sFree(stable);
    return EXIT_SUCCESS;
}
