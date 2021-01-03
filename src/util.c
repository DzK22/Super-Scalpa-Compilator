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

void printName (void) {
    printf("\t __                         __           _ \n");
    printf("\t/ _\\_   _ _ __   ___ _ __  / _\\ ___ __ _| |_ __   __ _\n");
    printf("\t\\ \\| | | | '_ \\ / _ \\ '__| \\ \\ / __/ _` | | '_ \\ / _` |\n");
    printf("\t_\\ \\ |_| | |_) |  __/ |    _\\ \\ (_| (_| | | |_) | (_| |\n");
    printf("\t\\__/\\__,_| .__/ \\___|_|    \\__/\\___\\__,_|_| .__/ \\__,_|\n");
    printf("\t         |_|                              |_|\n\n");

    printf("\t   ___                      _ _       _            \n");
    printf("\t  / __\\___  _ __ ___  _ __ (_) | __ _| |_ ___  _ __\n");
    printf("\t / /  / _ \\| '_ ` _ \\| '_ \\| | |/ _` | __/ _ \\| '__|\n");
    printf("\t/ /__| (_) | | | | | | |_) | | | (_| | || (_) | |\n");
    printf("\t\\____/\\___/|_| |_| |_| .__/|_|_|\\__,_|\\__\\___/|_|\n");
    printf("\t                     |_|\n\n");
}
