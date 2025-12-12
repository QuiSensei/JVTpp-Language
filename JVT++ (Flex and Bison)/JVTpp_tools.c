#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"

/* ----- Utility functions ----- */
int division_check(int numerator, int denominator) {
    if (denominator == 0) {
        // fprintf(stderr, "Error: Division by zero\n");
        return -1; // Indicate error
    }
    return 0;
}

char *strip_quotes(char *s) {
    size_t len = strlen(s);
    char *res = malloc(len - 1);

    strncpy(res, s + 1, len - 2);
    res[len - 2] = '\0';

    return res;
}