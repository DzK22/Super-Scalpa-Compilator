#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define COL_RESET "\e[0;m"
#define GREEN     "\e[38;2;100;200;60m"
#define PINK      "\e[38;2;250;20;160m"
#define YELLOW    "\e[38;2;255;250;0m"
#define CYAN      "\e[0;36m"

void ferr (int , char *s);

#endif
