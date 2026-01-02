#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "mips64.h"
#include "functions.h"

Symbol symbol_table[MAX_SYMBOLS];

size_t symbol_count;

int next_available_register;

char *open_source_file(const char *source_file) {
    FILE *source_code = fopen(source_file, "r");
    if(source_code == NULL) {
        fprintf(stderr, "ERROR: File '%s' not found or cannot be opened.\n", source_file);
        return NULL;
    }

    size_t buffer_size = 1024;
    size_t total_buffer_length = 0;

    char *lines_of_code = (char *)malloc(buffer_size * sizeof(char));
    if(lines_of_code == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        fclose(source_code);
        return NULL;
    }

    lines_of_code[0] = '\0';
    char lines[256];

    while(fgets(lines, sizeof(lines), source_code) != NULL) {
        size_t lines_length = strlen(lines);

        if(total_buffer_length + lines_length + 1 > buffer_size) {
            buffer_size *= 2;
            char *new_buffer = (char *)realloc(lines_of_code, buffer_size * sizeof(char));
            if(new_buffer == NULL) {
                fprintf(stderr, "Memory reallocation failed.\n");
                free(lines_of_code);
                fclose(source_code);
                return NULL;
            }
            lines_of_code = new_buffer;
        }

        strcat(lines_of_code, lines);
        total_buffer_length += lines_length;
    }

    fclose(source_code);
    return lines_of_code;
}

/*
 * ============================================
 * Function: add_variable
 * Purpose: Adds a new variable to the symbol table with register allocation
 * Parameters:
 *          name - variable name
 *          value - initial value
 *          initialized - flag indicating if variable has been initialized (1) or not (0)
 * ============================================
*/
void add_variable(const char *name, int value, int initialized) {
    // Check if symbol table is full
    if (symbol_count >= MAX_SYMBOLS) return;

    // Search for existing variable with the same name
    for(size_t i = 0; i < symbol_count; i++) {
        if(strcmp(symbol_table[i].name, name) == 0) {
            // Variable already exists - update its value and initialization status
            symbol_table[i].value = value;
            symbol_table[i].initialized = initialized;
            return;
        }
    }

    // Variable doesn't exist - add new entry to symbol table
    // Copy variable name (max 63 chars + null terminator)
    strncpy(symbol_table[symbol_count].name, name, 63);
    symbol_table[symbol_count].name[63] = '\0';  // Ensure null termination
    
    // Set variable properties
    symbol_table[symbol_count].value = value;
    symbol_table[symbol_count].initialized = initialized;
    symbol_table[symbol_count].mem_offset = symbol_count;  // Memory offset for assembly
    
    // Allocate a register for this variable
    symbol_table[symbol_count].reg_num = next_available_register;
    symbol_table[symbol_count].in_register = 0;  // Initially not in register
    next_available_register++;
    
    // Increment the symbol counter
    symbol_count++;
}

/* 
 * ============================================
 * Function: find_symbol
 * Purpose: Searches for a variable in the symbol table by name
 * Parameters: name - variable name to search for
 * Returns: Pointer to Symbol if found, NULL if not found
 * ============================================
*/
Symbol *find_symbol(const char *name) {
    // Linear search through symbol table
    for(size_t i = 0; i < symbol_count; i++) {
        if(strcmp(symbol_table[i].name, name) == 0) {
            // Found the variable - return pointer to its symbol table entry
            return &symbol_table[i];
        }
    }
    // Variable not found
    return NULL;
}

void white_space_trim(char *string) {
    if(!string) return;
    
    // Remove leading whitespace
    char *first_non_space = string;
    while (*first_non_space && isspace((unsigned char)*first_non_space)) {
        first_non_space++;
    }
    
    if (first_non_space != string) {
        memmove(string, first_non_space, strlen(first_non_space) + 1);
    }

    // Remove trailing whitespace
    char *last_char = string + strlen(string) - 1;
    while (last_char >= string && isspace((unsigned char)*last_char)) {
        *last_char = '\0';
        last_char--;
    }
}

/*
 * ============================================
 * Function: skip_escape_sequences_and_comments
 * Purpose: Advances the position index past whitespace and comments
 * Parameters:
 *   source_code - the source code string
 *   i - pointer to current position index (modified)
 *   line - pointer to current line number (modified)
 * ============================================
*/
void skip_escape_sequences_and_comments(const char *source_code, int *i, int *line) {
    // Continue looping until we hit non-whitespace, non-comment content
    while(source_code[*i] != '\0') {
        
        // ============================================
        // CASE 1: Skip whitespace (space, newline, tab, carriage return)
        // ============================================
        if(ESCAPE_SEQUENCE(source_code[*i])) {
            // Track line numbers when encountering newlines
            if(source_code[*i] == '\n') {
                (*line)++;
            }
            (*i)++;
        } 
        
        // ============================================
        // CASE 2: Skip single-line comments (//)
        // ============================================
        else if(source_code[*i] == '/' && source_code[(*i) + 1] == '/') {
            // Skip everything until end of line or end of file
            while(source_code[*i] != '\0' && source_code[*i] != '\n') {
                (*i)++;
            }
        } 
        
        // ============================================
        // CASE 3: Skip multi-line comments (/* ... */)
        // ============================================
        else if(source_code[*i] == '/' && source_code[(*i) + 1] == '*') {
            // Skip the opening "/*"
            (*i) += 2;
            
            // Continue until we find the closing "*/" or reach end of file
            while(source_code[*i] != '\0' && 
                  !(source_code[*i] == '*' && source_code[(*i) + 1] == '/')) {
                // Track line numbers within the comment block
                if(source_code[*i] == '\n') {
                    (*line)++;
                }
                (*i)++;
            }
            
            // Skip the closing "*/" if we found it
            if(source_code[*i] != '\0') {
                (*i) += 2;
            }
        } 
        
        // ============================================
        // Found actual code - stop skipping
        // ============================================
        else {
            break;
        }
    }
}

char *code_convertion(const char *source_file) {
    const char *keywords[] = {"develop[", "]finish"};
    int numKeywords = 2;

    // Create a working copy to process
    size_t source_len = strlen(source_file);

    char *result = (char *)malloc(source_len + 1);
    if(result == NULL) {
        fprintf(stderr, "ERROR: Memory allocation failed in code conversion.\n");
        return NULL;
    }
    strcpy(result, source_file);

    // Remove each keyword - use dynamic allocation for temp buffer
    for(int k = 0; k < numKeywords; k++) {
        char *temp = (char *)malloc(source_len * 2 + 1); // Extra space for safety
        if(temp == NULL) {
            fprintf(stderr, "ERROR: Memory allocation failed.\n");
            free(result);
            return NULL;
        }
        
        char *position;
        while((position = strstr(result, keywords[k])) != NULL) {
            *position = '\0';
            strcpy(temp, result);
            strcat(temp, position + strlen(keywords[k]));
            strcpy(result, temp);
        }
        free(temp);
    }

    // Process line by line and remove leading whitespace using white_space_trim
    size_t result_len = strlen(result);
    char *final = (char *)malloc(result_len * 2 + 1); // Extra space for converted lines
    if(final == NULL) {
        fprintf(stderr, "ERROR: Memory allocation failed.\n");
        free(result);
        return NULL;
    }
    final[0] = '\0';
    
    char line[1000];
    int i = 0, line_idx = 0;
    int len = strlen(result);
    
    while(i <= len) {
        if(result[i] == '\n' || result[i] == '\0') {
            line[line_idx] = '\0';
            
            // Use white_space_trim to remove leading and trailing whitespace
            white_space_trim(line);
            
            // Add the trimmed line
            if(strlen(line) > 0) {
                // Convert the line using datatype_convertions logic
                char *converted_line = dataype_convertion(line);
                if(converted_line != NULL) {
                    strcat(final, converted_line);
                    strcat(final, "\n");
                }
            } else if(result[i] == '\n') {
                strcat(final, "\n");
            }
            
            line_idx = 0;
            i++;
        } else {
            if(line_idx < 999) { // Prevent buffer overflow
                line[line_idx++] = result[i];
            }
            i++;
        }
    }

    free(result);

    return final;
}

char *dataype_convertion(const char *line) {
    static char c_code[1000];
    char line_copy[1000];
    
    // Safely copy input line
    strncpy(line_copy, line, 999);
    line_copy[999] = '\0';
    
    char datatype[50] = "";
    char variable[50] = "";
    char value[500] = "";
    
    // Check if line contains a declaration with "="
    if(strstr(line_copy, "=") != NULL) {
        // Parse the declaration: DATATYPE variable = value;
        char *token = strtok(line_copy, " ");
        if(token != NULL) {
            strncpy(datatype, token, 49);
            datatype[49] = '\0';
        }
        
        token = strtok(NULL, " ");
        if(token != NULL) {
            strncpy(variable, token, 49);
            variable[49] = '\0';
        }
        
        token = strtok(NULL, " ");  // Skip "="
        if(token == NULL || strcmp(token, "=") != 0) {
            // Invalid format
            strncpy(c_code, line, 999);
            c_code[999] = '\0';
            return c_code;
        }
        
        token = strtok(NULL, ";");   // Get value (everything until semicolon)
        if(token != NULL) {
            while(*token == ' ') token++;
            strncpy(value, token, 499);
            value[499] = '\0';
            
            int len = strlen(value);
            while(len > 0 && (value[len-1] == ' ' || value[len-1] == ';')) {
                value[--len] = '\0';
            }
        }
        
        // Convert datatype
        const char *c_datatype = NULL;
        if(strcmp(datatype, "NUMERAL") == 0) {
            c_datatype = "int";
        } else if(strcmp(datatype, "ALPHA") == 0) {
            c_datatype = "char*";
        } else if(strcmp(datatype, "DECIMAL") == 0) {
            c_datatype = "float";
        } else {
            strncpy(c_code, line, 999);
            c_code[999] = '\0';
            return c_code;
        }
        
        snprintf(c_code, sizeof(c_code), "%s %s = %s;", c_datatype, variable, value);
    } else {
        // Check for declaration without initialization (DATATYPE variable;)
        char *token = strtok(line_copy, " ");
        if(token != NULL) {
            strncpy(datatype, token, 49);
            datatype[49] = '\0';
        }
        
        token = strtok(NULL, ";");
        if(token != NULL) {
            while(*token == ' ') token++;
            strncpy(variable, token, 49);
            variable[49] = '\0';
            
            // Remove trailing spaces
            int len = strlen(variable);
            while(len > 0 && variable[len-1] == ' ') {
                variable[--len] = '\0';
            }
        }
        
        // Convert datatype
        const char *c_datatype = NULL;
        if(strcmp(datatype, "NUMERAL") == 0) {
            c_datatype = "int";
        } else if(strcmp(datatype, "ALPHA") == 0) {
            c_datatype = "char";
        } else if(strcmp(datatype, "DECIMAL") == 0) {
            c_datatype = "float";
        }
        
        if(c_datatype != NULL) {
            snprintf(c_code, sizeof(c_code), "%s %s;", c_datatype, variable);
        } else {
            strncpy(c_code, line, 999);
            c_code[999] = '\0';
        }
    }
    
    return c_code;
}

void compile_to_assemble(const char *source_code, const char *file_name) {
    // Open the output file for writing assembly code
    FILE *output_file = fopen(file_name, "w");
    if(output_file == NULL) {
        // Print error if file creation fails
        fprintf(stderr, "'%s' can't be created\n", file_name);
        return;
    }

    // Write the .data section header (where variables are declared)
    fprintf(output_file, ".data\n");
    
    /*
     * Create a duplicate of the source code for processing
     * This allows us to parse it multiple times without modifying the original
    */
    char *duplicated_source = strdup(source_code ? source_code : "");
    if(duplicated_source == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        fclose(output_file);
        return;
    }
    
    /* 
     * ============================================
     * FIRST PASS: Symbol Table Construction
     * ============================================
     * Scan through the source code to:
     * - Collect all variable declarations
     * - Build the symbol table
     * - Check for syntax/semantic errors
    */
    if(syntax_analyzer(duplicated_source, NULL) != 0) {
        // If errors were found, clean up and exit
        free(duplicated_source);
        fclose(output_file);
        return;
    }
    
    // Write all variable declarations to the .data section
    for(size_t i = 0; i < symbol_count; i++) {
        // Format: variable_name: .byte initial_value
        fprintf(output_file, "\t%s:\t.byte %d\n", symbol_table[i].name, symbol_table[i].value);
    }
    
    // Write the .code section header (where executable instructions go)
    fprintf(output_file, ".code\n");
    
    // Write the main entry point label
    fprintf(output_file, "main:\n");
    
    /*
     * ============================================
     * SECOND PASS: Code Generation
     * ============================================
     * Free the old duplicate and create a fresh copy of the source code
     * This resets our position in the source for the second pass
    */
    free(duplicated_source);

    duplicated_source = strdup(source_code ? source_code : "");
    if(duplicated_source == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        fclose(output_file);
        return;
    }
    
    // Process statements again, this time generating actual assembly code
    // The output_file is passed so assembly instructions are written
    if(syntax_analyzer(duplicated_source, output_file) != 0) {
        // If errors occurred during code generation, clean up and exit
        free(duplicated_source);
        fclose(output_file);
        return;
    }

    // Clean up: free allocated memory and close the output file
    free(duplicated_source);
    fclose(output_file);
}