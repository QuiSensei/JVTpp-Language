#ifndef FUNCTIONS_H
#define FUNCTIONS_H

/* 
 * Dangerous function-like macros
 * #define ESCAPE_SEQUENCE(c) ((c) == ' ' || (c) == '\n' || (c) == '\t' || (c) == '\r')
 * #define ALPHABETIC_CHARACTER(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || (c) == '_')
 * #define DIGIT_CHARACTER(c) ((c) >= '0' && (c) <= '9')
 * #define ALPHANUMERIC_CHARACTER(c) (ALPHABETIC_CHARACTER(c) || DIGIT_CHARACTER(c))
*/ 

// Safer version of function-like macros as static inline functions
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
    // Check for addition, subtraction, multiplication, or division
    return (c == '+' || c == '-' || c == '*' || c == '/');
}

// Maximum number of symbols (variables) that can be stored
#define MAX_SYMBOLS 128

// Structure to represent a variable/symbol in the symbol table
typedef struct {
    char name[64];          // Variable name (up to 63 characters + null terminator)
    int  value;             // Variable's value
    int  initialized;       // Flag: 1 if variable has been assigned a value, 0 otherwise
    int  mem_offset;        // Memory offset for the variable (used in assembly generation)
    int  reg_num;           // Allocated register number for this variable (-1 if not allocated)
    int  in_register;       // Flag: 1 if current value is in register, 0 if needs to be loaded
} Symbol;

// Global symbol table array to store all variables
extern Symbol symbol_table[MAX_SYMBOLS];

// Counter to track how many symbols are currently in the table
extern size_t symbol_count;

// Next available register for variable allocation (starts at r1, r0 is reserved)
extern int next_available_register;

// Function prototypes
char *open_source_file(const char *filename);

void add_variable(const char *name, int value, int initialized);
Symbol *find_symbol(const char *name);
void white_space_trim(char *s);

void skip_escape_sequences_and_comments(const char *source_code, int *i, int *line);

char *code_convertion(const char *source_file);
char *dataype_convertion(const char *line);

void compile_to_assemble(const char *source_code, const char *file_name);

#endif