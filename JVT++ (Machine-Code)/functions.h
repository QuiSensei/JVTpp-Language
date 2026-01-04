#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stddef.h>
#include "mips64.h"

// Character classification helpers
static inline int ESCAPE_SEQUENCE(int c) {
    return (c == ' ' || c == '\n' || c == '\t' || c == '\r');
}

static inline int ALPHABETIC_CHARACTER(int c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_'));
}

static inline int DIGIT_CHARACTER(int c) {
    return ((c >= '0' && c <= '9'));
}

static inline int ALPHANUMERIC_CHARACTER(int c) {
    return (ALPHABETIC_CHARACTER(c) || DIGIT_CHARACTER(c));
}

static inline int OPERATOR_CHARACTER(int c) {
    return (c == '+' || c == '-' || c == '*' || c == '/');
}

#define MAX_SYMBOLS 128

// Variable symbol table entry
typedef struct {
    char name[64];
    int  value;
    char str_value[256];
    int  data_type;      // 0=int, 1=char, 2=char*
    int  initialized;
    int  mem_offset;
    int  reg_num;        // -1 if not allocated
    int  in_register;
} Symbol;

extern Symbol symbol_table[MAX_SYMBOLS];
extern size_t symbol_count;
extern int next_available_register;

// File operations
char *open_source_file(const char *filename);

// Symbol table operations
void add_variable(const char *name, int value, const char *str_value, int data_type, int initialized);
Symbol *find_symbol(const char *name);

// String utilities
void white_space_trim(char *s);
void skip_escape_sequences_and_comments(const char *source_code, int *i, int *line);
char *extract_string_literal(const char *str);

// Code conversion
char *code_convertion(const char *source_file);
char *dataype_convertion(const char *line);

// Compilation stages
void compile_to_assemble(const char *source_code, const char *file_name);
int lexical_analyzer(const char *expr, FILE *output_file, int target_reg);
int syntax_analyzer(const char *source_code, FILE *output_file);
void make_assembly_for_statement(FILE *output_file, const char *statement);

#endif