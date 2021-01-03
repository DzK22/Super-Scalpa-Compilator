#include "../headers/util.h"

// put here all global functions
void arrShuffle(char **array, int size) {
    if (size > 1) {
        int i;
        for (i = 0; i < size - 1; i++) {
          size_t j = i + rand() / (RAND_MAX / (size - i) + 1);
          char *t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}
