%{
    #include <stdio.h>
    #include <ctype.h>
    #include "../headers/stable.h"
    #include "../headers/quad.h"
    #include "../tmp/cs.tab.h"
    int linecpt = 1;
%}

A                  [aA]
B                  [bB]
C                  [cC]
D                  [dD]
E                  [eE]
F                  [fF]
G                  [gG]
H                  [hH]
I                  [iI]
J                  [jJ]
K                  [kK]
L                  [lL]
M                  [mM]
N                  [nN]
O                  [oO]
P                  [pP]
Q                  [qQ]
R                  [rR]
S                  [sS]
T                  [tT]
U                  [uU]
V                  [vV]
W                  [wW]
X                  [xX]
Y                  [yY]
Z                  [zZ]

digit              [0-9]
letter             [a-zA-Z]
ident              {letter}("'"|"_"|{letter}|{digit})*

cst_int            {digit}+
cst_bool           "true"|"false"
cst_string         ["][^\"\n]*["]

atomic_type        {type_int}|{type_bool}|{type_unit}
comment            \(\*([^*]|\*+[^*)]|\n)*\*+\)

%%

"\n"                            { linecpt ++;                       }

{P}{R}{O}{G}{R}{A}{M}           { return PROGRAM_;                  }

{W}{R}{I}{T}{E}                 { return WRITE_;                    }

{R}{E}{A}{D}                    { return READ_;                     }

{B}{E}{G}{I}{N}                 { return BEGIN_;                    }

{E}{N}{D}                       { return END_;                      }

{V}{A}{R}                       { return VAR_;                      }

{R}{E}{T}{U}{R}{N}              { return RETURN_;                   }

{R}{E}{F}                       { return REF_;                      }

{W}{H}{I}{L}{E}                 { return WHILE_;                    }

{D}{O}                          { return DO_;                       }

{I}{F}                          { return IF_;                       }

{T}{H}{E}{N}                    { return THEN_;                     }

{E}{L}{S}{E}                    { return ELSE_;                     }

{A}{N}{D}                       { return AND_;                      }

{O}{R}                          { return OR_;                       }

{X}{O}{R}                       { return XOR_;                      }

{N}{O}{T}                       { return NOT_;                      }

{I}{N}{T}                       { return INT_;                      }

{B}{O}{O}{L}                    { return BOOL_;                     }

{U}{N}{I}{T}                    { return UNIT_;                     }

{S}{T}{R}{I}{N}{G}              { return STRING_;                   }

{A}{R}{R}{A}{Y}                 { return ARRAY_;                    }

{O}{F}                          { return OF_;                       }

{F}{U}{N}{C}{T}{I}{O}{N}        { return FUNCTION_;                 }

" "*                            {}

{cst_int}                       { yylval.cte.type = S_INT;
                                  yylval.cte.ival = atoi(yytext);
                                  return CTE_;                      }

{cst_bool}                      { yylval.cte.type = S_BOOL;
                                  yylval.cte.bval = strcmp(yytext, "true") ? false : true;
                                  return CTE_;                      }

{cst_string}                    { yylval.cte.type = S_STRING;
                                  yylval.cte.sval = strdup(yytext);
                                  return CTE_;                      }

{comment}                       { /* ignore comments*/              }

":"                             { return DPOINT_;                   }

"+"                             { return PLUS_;                     }

"-"                             { return MINUS_;                    }

"*"                             { return MULT_;                     }

"/"                             { return DIV_;                      }

"^"                             { return EXP_;                      }

"<"                             { return INF_;                      }

"<="                            { return INF_EQ_;                   }

">"                             { return SUP_;                      }

">="                            { return SUP_EQ_;                   }

"="                             { return EQUAL_;                    }

"<>"                            { return DIFF_;                     }

":="                            { return AFFEC_;                    }

".."                            { return TWO_POINTS_;               }

";"                             { return DOTCOMMA_;                 }

","                             { return COMMA_;                    }

"("                             { return PARLEFT_;                  }

")"                             { return PARRIGHT_;                 }

"["                             { return BRALEFT_;                  }

"]"                             { return BRARIGHT_;                 }

{ident}                         { yylval.sval = strdup(yytext);
                                  return IDENT_;                    }

.                               { fprintf(stderr, "Unrecognized character : %s at line %d\n", yytext, linecpt);
                                  return EXIT_FAILURE;              }

%%

void freeLex (void) {
    yy_delete_buffer(YY_CURRENT_BUFFER);
    free(yy_buffer_stack);
}
