%{

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
int yyerror (char *s);
int yylex (void);


%}

%%

liste :  { printf("COUCOU DANYL\n"); }

%%

int main (void)
{
  return yyparse();
}

int yyerror (char *s)
{
  fprintf(stderr, "Error: %s\n", s);
  exit(1);
}

int yywrap (void)
{
  return 1;
}

