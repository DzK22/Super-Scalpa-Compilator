%{
    #include <stdio.h>
    #include <ctype.h>
    #include "../headers/stable.h"
    #include "../tmp/cs.tab.h"
    int linecpt = 1;
%}
A [aA]
B [bB]
C [cC]
D [dD]
E [eE]
F [fF]
G [gG]
H [hH]
I [iI]
J [jJ]
K [kK]
L [lL]
M [mM]
N [nN]
O [oO]
P [pP]
Q [qQ]
R [rR]
S [sS]
T [tT]
U [uU]
V [vV]
W [wW]
X [xX]
Y [yY]
Z [zZ]
digit     [0-9]
letter    [a-zA-Z]
ident     {letter}("'"|"_"|{letter}|{digit})*
cst_int   {digit}+
cst_bool  "true"|"false"
cst_string  "[^\"]+"
parens    [()]
hooks     [\[\]]
newline   \n
int  "int"
bool "bool"
unit "unit"
atomic_type      {type_int}|{type_bool}|{type_unit}

%%

{newline}                                   { linecpt++; }

{P}{R}{O}{G}{R}{A}{M}                       { return PROGRAM_; }

{W}{R}{I}{T}{E}                             { return WRITE_; }

{R}{E}{A}{D}                                { return READ_; }

{B}{E}{G}{I}{N}                             { return BEGIN_; }

{E}{N}{D}                                   { return END_; }

{V}{A}{R}                                   { return VAR_; }

{R}{E}{T}{U}{R}{N}                          { return RETURN_; }

{R}{E}{F}                                   { return REF_;}

{int}                                  { return INT_; }

{bool}                                 { return BOOL_; }

{unit}                                 { return UNIT_;}


" "*                                        {}

{cst_int}                                   { yylval.val = atoi(yytext);
                                              return INTEGER_; }

{cst_bool}                                  { yylval.bol = strcmp(yytext, "true") ? false : true;
                                              return BOOLEAN_;}

{cst_string}                                  { yylval.str = strdup(yytext);
                                              return STRING_;}
                                              
{ident}                                     { yylval.str = strdup(yytext);
                                              return IDENT_; }

":"                                         { return yytext[0]; }

"+"                                         { return PLUS_; }

"-"                                         { return MINUS_; }

"*"                                         { return MULT_; }

"/"                                         { return DIV_; }

"^"                                         { return EXP_; }

"<"                                         { return INF_; }

"<="                                        { return INF_EQ_; }

">"                                         { return SUP_; }

">="                                        { return SUP_EQ_; }

"="                                         { return EQUAL_; }

"<>"                                        { return DIFF_; }

":="                                        { return AFFEC_; }

";"                                        { return DOTCOMMA_; }

","                                        { return COMMA_; }

{parens}                                    { return yytext[0]; }

{hooks}                                     { return yytext[0]; }

.                                           { fprintf(stderr, "Unrecognized character : %s at line %d\n", yytext, linecpt);
                                              return EXIT_FAILURE; }

%%

void freeLex (void) {
    yy_delete_buffer(YY_CURRENT_BUFFER);
    free(yy_buffer_stack);
}
