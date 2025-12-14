#ifndef TOOLS_H
#define TOOLS_H

#include <stdlib.h>
#include <string.h>

/* ----- Symbol Table Structure ----- */
#define MAX_SYMBOLS 100

typedef struct {
    char *name;
    int value;
    int is_string;
    char *str_value;
} Symbol;

/* ----- Utility Function Prototypes ----- */
int division_check(int numerator, int denominator);
char *strip_quotes(char *s);

// FIXED: Declare global symbol table and counter
extern Symbol symbol_table[MAX_SYMBOLS];
extern int symbol_count;

/* ----- Symbol Table Function Prototypes ----- */
int lookup_symbol(char *name);
void add_symbol(char *name, int value);
void add_string_symbol(char *name, char *value);
int get_symbol_value(char *name);
char *get_symbol_string(char *name);

/* ----- printing Functions ----- */
void print_fstring(char *format_str);
void println_fstring(char *format_str);

#endif