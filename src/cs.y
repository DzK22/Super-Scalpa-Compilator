%{
    #include <stdio.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <math.h>
    #include "../headers/stable.h"
    #include "../headers/quad.h"
    #include "../headers/mips.h"
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
    char *tid;
    int  val;
    bool bol;
    char *str;

    struct symbol *s;
    stype type;
    struct {
        struct symbol   *res;
        struct quad     *code;
    } gencode; // Pour les expressions

    struct li {
      struct li *next;
      char *id;
    } list;

    struct typ {
      stype type;
      char cha;
      int val;
      bool bol;
    } cte_type;
}

%token PROGRAM_ IDENT_ NEWLINE_ END_ WRITE_  MULT_ DIV_ PLUS_ MINUS_ EXP_ INF_ INF_EQ_ BEGIN_ READ_
%token SUP_ SUP_EQ_ EQUAL_ DIFF_ AFFEC_ AND_ OR_ XOR_ NOT_ INT_ BOOL_ UNIT_ VAR_ RETURN_ REF_ CTE_ DOTCOMMA_ COMMA_

%type   <cte_type> CTE_
%type   <tid> IDENT_
%type   <gencode>  expr instr program
%type   <list>  vardecllist fundecllist identlist varsdecl fundecl lvalue sequence

%left   PLUS_ MINUS_
%left   MULT_ DIV_
%right  EXP_
%left   NOT_

%start  program
%%

program: PROGRAM_ IDENT_ vardecllist fundecllist instr  { }
    ;

vardecllist : %empty                       {  }
           | varsdecl                      {  }
           | varsdecl DOTCOMMA_ vardecllist      {  }
           ;

varsdecl: VAR_ identlist ':' typename {  }
          ;

identlist : IDENT_  {  }
         | IDENT_ COMMA_ identlist  {  }
         ;

typename : atomictype {}
         ;

atomictype : UNIT_ {}
           | BOOL_ {}
           | INT_ {}
           ;

fundecllist : %empty                        {  }
            | fundecl DOTCOMMA_ fundecllist       {  }
           ;
/* REGLE BIDON , A SUPPRIMER APRES mdr */
fundecl : %empty                            {}
        ;

instr: lvalue AFFEC_ expr   {}
      | RETURN_ expr {}
      | BEGIN_ sequence END_ {}
      | BEGIN_ END_ {}
      | WRITE_ expr {}
      ;

sequence : instr DOTCOMMA_ sequence  {}
         | instr DOTCOMMA_ {}
         | instr  {}
        ;

lvalue: IDENT_ {}
      ;

exprlist : expr {}
        |  expr COMMA_ exprlist {}
        ;

expr : CTE_                                  { }
      | '(' expr ')'                          { }
      | expr opb expr                         { }
      | opu expr                              { }
      | IDENT_                                { }
      | IDENT_ '(' exprlist ')'                { }
      ;

opb : MULT_                                    {}
    | DIV_                                     {}
    | PLUS_                                    {}
    | MINUS_                                   {}
    | EXP_                                     {}
    | INF_                                     {}
    | INF_EQ_                                  {}
    | SUP_                                     {}
    | SUP_EQ_                                  {}
    | EQUAL_                                   {}
    | DIFF_                                    {}
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
         yydebug = 1;
    #endif
	yyin = fopen(argv[1], "r");
	return yyparse();

}
