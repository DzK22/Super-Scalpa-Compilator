#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#define COL_RESET "\e[0;m"
#define GREEN     "\e[38;2;100;200;60m"
#define PINK      "\e[38;2;250;20;160m"
#define YELLOW    "\e[38;2;255;250;0m"
#define CYAN      "\e[0;36m"
#define PURPLE    "\e[0;35m"

void arrShuffle(char **array, int size);

#define ferr(str) { \
    fprintf(stderr, "Error in %s at line %d : %s\n", __FILE__, __LINE__, str); \
    exit(EXIT_FAILURE); }

// snprintf test
#define snpt(res) \
    if (res < 0 || res >= LEN) \
        ferr("snprintf");

#endif
