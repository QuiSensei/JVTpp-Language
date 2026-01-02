#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// Structure to represent a variable/symbol in the symbol table
typedef struct {
    char name[64];          // Variable name (up to 63 characters + null terminator)
    int  value;             // Variable's value
    int  initialized;       // Flag: 1 if variable has been assigned a value, 0 otherwise
    int  mem_offset;        // Memory offset for the variable (used in assembly generation)
    int  reg_num;           // Allocated register number for this variable (-1 if not allocated)
    int  in_register;       // Flag: 1 if current value is in register, 0 if needs to be loaded
} Symbol;

// Function prototypes
char *open_source_file(const char *filename);

void add_variable(const char *name, int value, int initialized);
int is_operator(char c);
Symbol *find_symbol(const char *name);
void white_space_trim(char *s);

void skip_escape_sequences_and_comments(const char *source_code, int *i, int *line);

void code_convertion(const char *source_file);
char *dataype_convertion(const char *line);

#endif