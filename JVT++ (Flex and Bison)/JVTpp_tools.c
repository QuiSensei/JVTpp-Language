#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"

/* ----- Global Symbol Table ----- */
// FIXED: Define the global symbol table
Symbol symbol_table[MAX_SYMBOLS];
int symbol_count = 0;

/* ----- Utility Functions ----- */
int division_check(int numerator, int denominator) {
    if (denominator == 0) {
        return -1; // Indicate error
    }
    return 0;
}

char *strip_quotes(char *s) {
    if (s == NULL) return NULL;
    
    size_t len = strlen(s);
    if (len < 2) return strdup(s);
    
    char *res = malloc(len - 1);
    if (res == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    strncpy(res, s + 1, len - 2);
    res[len - 2] = '\0';

    return res;
}

/* ----- Symbol Table Functions ----- */
int lookup_symbol(char *name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void add_symbol(char *name, int value) {
    int idx = lookup_symbol(name);
    if (idx == -1) {
        if (symbol_count >= MAX_SYMBOLS) {
            fprintf(stderr, "Error: Symbol table full\n");
            return;
        }
        symbol_table[symbol_count].name = strdup(name);
        symbol_table[symbol_count].value = value;
        symbol_table[symbol_count].is_string = 0;
        symbol_table[symbol_count].str_value = NULL;
        symbol_count++;
    } else {
        symbol_table[idx].value = value;
        symbol_table[idx].is_string = 0;
    }
}

void add_string_symbol(char *name, char *value) {
    int idx = lookup_symbol(name);
    if (idx == -1) {
        if (symbol_count >= MAX_SYMBOLS) {
            fprintf(stderr, "Error: Symbol table full\n");
            return;
        }
        symbol_table[symbol_count].name = strdup(name);
        symbol_table[symbol_count].str_value = strdup(value);
        symbol_table[symbol_count].is_string = 1;
        symbol_table[symbol_count].value = 0;
        symbol_count++;
    } else {
        if (symbol_table[idx].is_string && symbol_table[idx].str_value) {
            free(symbol_table[idx].str_value);
        }
        symbol_table[idx].str_value = strdup(value);
        symbol_table[idx].is_string = 1;
    }
}

int get_symbol_value(char *name) {
    int idx = lookup_symbol(name);
    if (idx != -1 && !symbol_table[idx].is_string) {
        return symbol_table[idx].value;
    }
    fprintf(stderr, "Error: Undefined variable '%s' or type mismatch\n", name);
    return 0;
}

char *get_symbol_string(char *name) {
    int idx = lookup_symbol(name);
    if (idx != -1 && symbol_table[idx].is_string) {
        return symbol_table[idx].str_value;
    }
    return NULL;
}