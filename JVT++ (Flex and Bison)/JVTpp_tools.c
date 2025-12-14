#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tools.h"

/* ----- Global Symbol Table ----- */
Symbol symbol_table[MAX_SYMBOLS];
int symbol_count = 0;

/* ----- Utility Functions ----- */
int division_check(int numerator, int denominator) {
    if (denominator == 0) {
        return -1;
    }
    return 0;
}

char *strip_quotes(char *s) {
    if (s == NULL) return NULL;
    
    size_t len = strlen(s);
    if (len < 2) return strdup(s);
    
    char *res = malloc(len - 1);
    if (res == NULL) {
        fprintf(stderr, "ERROR: Memory allocation failed\n");
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
    } else {
        fprintf(stderr, "Error: Undefined variable '%s' or type mismatch\n", name);
        exit(1); 
    }
    return 0;
}

char *get_symbol_string(char *name) {
    int idx = lookup_symbol(name);
    if (idx != -1 && symbol_table[idx].is_string) {
        return symbol_table[idx].str_value;
    }
    return NULL;
}

/* ----- F-String Functions ----- */
void print_fstring(char *format_str) {
    char *str = strip_quotes(format_str);
    char output[1024] = "";
    int out_pos = 0;
    int i = 0;
    
    while (str[i] != '\0') {
        if (str[i] == '{') {
            i++;
            
            if (str[i] == '}') {
                i++;
                continue;
            }
            
            char var_name[100] = "";
            int var_pos = 0;
            while (str[i] != '}' && str[i] != '\0') {
                var_name[var_pos++] = str[i++];
            }
            var_name[var_pos] = '\0';
            
            if (str[i] == '}') {
                i++;
            }
            
            // Trim whitespace
            int start = 0;
            while (isspace(var_name[start])) start++;
            int end = strlen(var_name) - 1;
            while (end >= start && isspace(var_name[end])) end--;
            
            char trimmed[100] = "";
            int j = 0;
            for (int k = start; k <= end; k++) {
                trimmed[j++] = var_name[k];
            }
            trimmed[j] = '\0';
            
            int idx = lookup_symbol(trimmed);
            if (idx != -1) {
                if (symbol_table[idx].is_string) {
                    out_pos += sprintf(output + out_pos, "%s", symbol_table[idx].str_value);
                } else {
                    out_pos += sprintf(output + out_pos, "%d", symbol_table[idx].value);
                }
            } else {
                out_pos += sprintf(output + out_pos, "{undefined:%s}", trimmed);
            }
        } else if (str[i] == '\\' && str[i+1] == 'n') {
            output[out_pos++] = '\n';
            i += 2;
        } else if (str[i] == '\\' && str[i+1] == 't') {
            output[out_pos++] = '\t';
            i += 2;
        } else if (str[i] == '\\' && str[i+1] == '{') {
            output[out_pos++] = '{';
            i += 2;
        } else if (str[i] == '\\' && str[i+1] == '}') {
            output[out_pos++] = '}';
            i += 2;
        } else {
            output[out_pos++] = str[i++];
        }
    }
    output[out_pos] = '\0';
    
    printf("%s", output);
    free(str);
}

void println_fstring(char *format_str) {
    print_fstring(format_str);
    printf("\n");
}
