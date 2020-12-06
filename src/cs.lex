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
cst_char  \'[^\']+\'
parens    [()]
hooks     [\[\]]
newline   \n
type_int  "int"
type_str  "string"
type      {type_int}|{type_str}
var       "var"

%%

{P}{R}{O}{G}{R}{A}{M}                       { return PROGRAM; }

{W}{R}{I}{T}{E}                             { return WRITE; }

{E}{N}{D}                                   { return END; }

{newline}                                   { linecpt++; }

" "*                                        {}

{type_int}                                  { yylval.type = S_INTEGER;
                                              return TYPE; }

{type_str}                                  { yylval.type = S_STRING;
                                              return TYPE; }

{var}                                       { return VAR; }

":"                                         { return yytext[0]; }

{ident}                                     { yylval.tid = strdup(yytext);
                                                return ID; }

"+"                                         { return PLUS; }

"-"                                         { return MINUS; }

"*"                                         { return MULT; }

"/"                                         { return DIV; }

"^"                                         { return EXP; }

"<"                                         { return INF; }

"<="                                        { return INF_EQ; }

">"                                         { return SUP; }

">="                                        { return SUP_EQ; }

"="                                         { return EQUAL; }

"<>"                                        { return DIFF; }

":="                                        { return AFFEC; }

{parens}                                    { return yytext[0]; }

{hooks}                                     { return yytext[0]; }

{cst_int}                                   { yylval.val = atoi(yytext);
                                              return INTEGER; }

.                                           { fprintf(stderr, "Unrecognized character : %s at line %d\n", yytext, linecpt);
                                              return EXIT_FAILURE; }

%%

void freeLex (void) {
    yy_delete_buffer(YY_CURRENT_BUFFER);
    free(yy_buffer_stack);
}
