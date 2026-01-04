#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "mips64.h"
#include "functions.h"

Symbol symbol_table[MAX_SYMBOLS];
size_t symbol_count;
int next_available_register = 1;

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

void add_variable(const char *name, int value, const char *str_value, int data_type, int initialized) {
    if (symbol_count >= MAX_SYMBOLS) return;

    for(size_t i = 0; i < symbol_count; i++) {
        if(strcmp(symbol_table[i].name, name) == 0) {
            symbol_table[i].value = value;
            symbol_table[i].initialized = initialized;
            symbol_table[i].data_type = data_type;
            
            if(str_value && (data_type == 1 || data_type == 2)) {
                strncpy(symbol_table[i].str_value, str_value, 255);
                symbol_table[i].str_value[255] = '\0';
            }
            return;
        }
    }

    strncpy(symbol_table[symbol_count].name, name, 63);
    symbol_table[symbol_count].name[63] = '\0';
    
    symbol_table[symbol_count].value = value;
    symbol_table[symbol_count].data_type = data_type;
    symbol_table[symbol_count].initialized = initialized;
    symbol_table[symbol_count].mem_offset = symbol_count * 8;
    
    if(str_value && (data_type == 1 || data_type == 2)) {
        strncpy(symbol_table[symbol_count].str_value, str_value, 255);
        symbol_table[symbol_count].str_value[255] = '\0';
    } else {
        symbol_table[symbol_count].str_value[0] = '\0';
    }
    
    symbol_table[symbol_count].reg_num = next_available_register;
    symbol_table[symbol_count].in_register = 0;
    next_available_register++;
    if(next_available_register > 25) next_available_register = 8;
    
    symbol_count++;
}

Symbol *find_symbol(const char *name) {
    for(size_t i = 0; i < symbol_count; i++) {
        if(strcmp(symbol_table[i].name, name) == 0) {
            return &symbol_table[i];
        }
    }
    return NULL;
}

void white_space_trim(char *string) {
    if(!string) return;
    
    char *first_non_space = string;
    while (*first_non_space && isspace((unsigned char)*first_non_space)) {
        first_non_space++;
    }
    
    if (first_non_space != string) {
        memmove(string, first_non_space, strlen(first_non_space) + 1);
    }

    char *last_char = string + strlen(string) - 1;
    while (last_char >= string && isspace((unsigned char)*last_char)) {
        *last_char = '\0';
        last_char--;
    }
}

void skip_escape_sequences_and_comments(const char *source_code, int *i, int *line) {
    while(source_code[*i] != '\0') {
        if(ESCAPE_SEQUENCE(source_code[*i])) {
            if(source_code[*i] == '\n') {
                (*line)++;
            }
            (*i)++;
        } 
        else if(source_code[*i] == '/' && source_code[(*i) + 1] == '/') {
            while(source_code[*i] != '\0' && source_code[*i] != '\n') {
                (*i)++;
            }
        } 
        else if(source_code[*i] == '/' && source_code[(*i) + 1] == '*') {
            (*i) += 2;
            while(source_code[*i] != '\0' && 
                  !(source_code[*i] == '*' && source_code[(*i) + 1] == '/')) {
                if(source_code[*i] == '\n') {
                    (*line)++;
                }
                (*i)++;
            }
            if(source_code[*i] != '\0') {
                (*i) += 2;
            }
        } 
        else {
            break;
        }
    }
}

char *code_convertion(const char *source_file) {
    const char *keywords[] = {"develop[", "]finish"};
    int numKeywords = 2;

    size_t source_len = strlen(source_file);

    char *result = (char *)malloc(source_len + 1);
    if(result == NULL) {
        fprintf(stderr, "ERROR: Memory allocation failed in code conversion.\n");
        return NULL;
    }
    strcpy(result, source_file);

    for(int k = 0; k < numKeywords; k++) {
        char *temp = (char *)malloc(source_len * 2 + 1);
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

    size_t result_len = strlen(result);
    char *final = (char *)malloc(result_len * 2 + 1);
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
            white_space_trim(line);
            
            if(strlen(line) > 0) {
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
            if(line_idx < 999) {
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
    
    strncpy(line_copy, line, 999);
    line_copy[999] = '\0';
    
    char datatype[50] = "";
    char variable[50] = "";
    char value[500] = "";
    
    if(strstr(line_copy, "=") != NULL) {
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
        
        token = strtok(NULL, " ");
        if(token == NULL || strcmp(token, "=") != 0) {
            strncpy(c_code, line, 999);
            c_code[999] = '\0';
            return c_code;
        }
        
        token = strtok(NULL, ";");
        if(token != NULL) {
            while(*token == ' ') token++;
            strncpy(value, token, 499);
            value[499] = '\0';
            
            int len = strlen(value);
            while(len > 0 && (value[len-1] == ' ' || value[len-1] == ';')) {
                value[--len] = '\0';
            }
        }
        
        const char *c_datatype = NULL;
        if(strcmp(datatype, "NUMERAL") == 0) {
            c_datatype = "int";
            snprintf(c_code, sizeof(c_code), "%s %s = %s;", c_datatype, variable, value);
        } else if(strcmp(datatype, "ALPHA") == 0) {
            c_datatype = "char";
            snprintf(c_code, sizeof(c_code), "%s %s[] = %s;", c_datatype, variable, value);
        } else {
            strncpy(c_code, line, 999);
            c_code[999] = '\0';
            return c_code;
        }
    } else {
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
            
            char *bracket = strchr(variable, '[');
            if(bracket) *bracket = '\0';

            int len = strlen(variable);
            while(len > 0 && variable[len-1] == ' ') {
                variable[--len] = '\0';
            }
        }
        
        const char *c_datatype = NULL;
        if(strcmp(datatype, "NUMERAL") == 0) {
            c_datatype = "int";
        } else if(strcmp(datatype, "ALPHA") == 0) {
            c_datatype = "char";
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

char *extract_string_literal(const char *str) {
    static char buffer[256];
    buffer[0] = '\0';
    
    if(!str) return buffer;
    
    const char *start = strchr(str, '"');
    if(!start) return buffer;
    
    start++;
    const char *end = strchr(start, '"');
    if(!end) return buffer;
    
    int len = end - start;
    if(len > 255) len = 255;
    
    strncpy(buffer, start, len);
    buffer[len] = '\0';
    
    return buffer;
}

void compile_to_assemble(const char *source_code, const char *file_name) {
    FILE *output_file = fopen(file_name, "w");
    if(output_file == NULL) {
        fprintf(stderr, "'%s' can't be created\n", file_name);
        return;
    }

    fprintf(output_file, ".data\n");
    
    char *duplicated_source = strdup(source_code ? source_code : "");
    if(duplicated_source == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        fclose(output_file);
        return;
    }
    
    if(syntax_analyzer(duplicated_source, NULL) != 0) {
        free(duplicated_source);
        fclose(output_file);
        return;
    }
    
    for(size_t i = 0; i < symbol_count; i++) {
        if(symbol_table[i].data_type == 0) {
            fprintf(output_file, "\t%s:\t.word %d\n", symbol_table[i].name, symbol_table[i].value);
        } else if(symbol_table[i].data_type == 1 || symbol_table[i].data_type == 2) {
            fprintf(output_file, "\t%s:\t.asciiz \"%s\"\n", symbol_table[i].name, symbol_table[i].str_value);
        }
    }
    
    fprintf(output_file, ".text\n");
    fprintf(output_file, "main:\n");
    
    free(duplicated_source);

    duplicated_source = strdup(source_code ? source_code : "");
    if(duplicated_source == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        fclose(output_file);
        return;
    }
    
    if(syntax_analyzer(duplicated_source, output_file) != 0) {
        free(duplicated_source);
        fclose(output_file);
        return;
    }

    fprintf(output_file, "\n\tsyscall 0\n");

    free(duplicated_source);
    fclose(output_file);
}

int lexical_analyzer(const char *expr, FILE *output_file, int target_reg) {
    if(!output_file) return -1;
    
    char tokens[64][64];
    int token_count = 0;
    int i = 0;
    
    while(expr[i] != '\0' && token_count < 64) {
        while(isspace(expr[i])) i++;
        if(expr[i] == '\0') break;
        
        if(ALPHABETIC_CHARACTER(expr[i])) {
            int j = 0;
            while(ALPHANUMERIC_CHARACTER(expr[i]) && j < 63) {
                tokens[token_count][j++] = expr[i++];
            }
            tokens[token_count][j] = '\0';
            token_count++;
        } 
        else if(DIGIT_CHARACTER(expr[i])) {
            int j = 0;
            while(DIGIT_CHARACTER(expr[i]) && j < 63) {
                tokens[token_count][j++] = expr[i++];
            }
            tokens[token_count][j] = '\0';
            token_count++;
        } 
        else if(OPERATOR_CHARACTER(expr[i])) {
            tokens[token_count][0] = expr[i];
            tokens[token_count][1] = '\0';
            token_count++;
            i++;
        } 
        else {
            i++;
        }
    }
    
    if(token_count == 0) return -1;
    
    const char *r0_name = get_register_name(0);
    
    if(token_count == 1) {
        if(DIGIT_CHARACTER(tokens[0][0])) {
            const char *target_reg_name = get_register_name(target_reg);
            fprintf(output_file, "\tdaddiu %s, %s, %s\n", target_reg_name, r0_name, tokens[0]);
        } 
        else {
            Symbol *sym = find_symbol(tokens[0]);
            if(sym && sym->reg_num >= 0) {
                const char *target_reg_name = get_register_name(target_reg);
                fprintf(output_file, "\tld %s, %s(%s)\n", target_reg_name, sym->name, r0_name);
                return target_reg;
            }
        }
        return target_reg;
    }
    
    if(token_count == 3) {
        int reg1 = target_reg + 10;
        int reg2 = target_reg + 11;
        
        if(reg1 > 25) reg1 = 8;
        if(reg2 > 25) reg2 = 9;
        
        if(DIGIT_CHARACTER(tokens[0][0])) {
            const char *reg1_name = get_register_name(reg1);
            fprintf(output_file, "\tdaddiu %s, %s, %s\n", reg1_name, r0_name, tokens[0]);
        } else {
            Symbol *sym = find_symbol(tokens[0]);
            if(sym && sym->reg_num >= 0) {
                const char *reg1_name = get_register_name(reg1);
                fprintf(output_file, "\tld %s, %s(%s)\n", reg1_name, sym->name, r0_name);
            }
        }
        
        if(DIGIT_CHARACTER(tokens[2][0])) {
            const char *reg2_name = get_register_name(reg2);
            fprintf(output_file, "\tdaddiu %s, %s, %s\n", reg2_name, r0_name, tokens[2]);
        } else {
            Symbol *sym = find_symbol(tokens[2]);
            if(sym && sym->reg_num >= 0) {
                const char *reg2_name = get_register_name(reg2);
                fprintf(output_file, "\tld %s, %s(%s)\n", reg2_name, sym->name, r0_name);
            }
        }
        
        const char *target_reg_name = get_register_name(target_reg);
        const char *reg1_name = get_register_name(reg1);
        const char *reg2_name = get_register_name(reg2);
        
        char op = tokens[1][0];
        if(op == '+') {
            fprintf(output_file, "\tdaddu %s, %s, %s\n", target_reg_name, reg1_name, reg2_name);
        } else if(op == '-') {
            fprintf(output_file, "\tdsubu %s, %s, %s\n", target_reg_name, reg1_name, reg2_name);
        } else if(op == '*') {
            fprintf(output_file, "\tdmult %s, %s\n", reg1_name, reg2_name);
            fprintf(output_file, "\tmflo %s\n", target_reg_name);
        } else if(op == '/') {
            fprintf(output_file, "\tddiv %s, %s\n", reg1_name, reg2_name);
            fprintf(output_file, "\tmflo %s\n", target_reg_name);
        }
        
        return target_reg;
    }
    
    int current_reg = target_reg;
    
    if(DIGIT_CHARACTER(tokens[0][0])) {
        const char *current_reg_name = get_register_name(current_reg);
        fprintf(output_file, "\tdaddiu %s, %s, %s\n", current_reg_name, r0_name, tokens[0]);
    } else {
        Symbol *sym = find_symbol(tokens[0]);
        if(sym && sym->reg_num >= 0) {
            const char *current_reg_name = get_register_name(current_reg);
            fprintf(output_file, "\tld %s, %s(%s)\n", current_reg_name, sym->name, r0_name);
        }
    }
    
    for(int j = 1; j < token_count - 1; j += 2) {
        char op = tokens[j][0];
        char *next_operand = tokens[j + 1];
        
        int operand_reg = target_reg + 1;
        if(operand_reg > 25) operand_reg = 9;
        
        if(DIGIT_CHARACTER(next_operand[0])) {
            const char *operand_reg_name = get_register_name(operand_reg);
            fprintf(output_file, "\tdaddiu %s, %s, %s\n", operand_reg_name, r0_name, next_operand);
        } else {
            Symbol *sym = find_symbol(next_operand);
            if(sym && sym->reg_num >= 0) {
                const char *operand_reg_name = get_register_name(operand_reg);
                fprintf(output_file, "\tld %s, %s(%s)\n", operand_reg_name, sym->name, r0_name);
            }
        }
        
        const char *current_reg_name = get_register_name(current_reg);
        const char *operand_reg_name = get_register_name(operand_reg);
        
        if(op == '+') {
            fprintf(output_file, "\tdaddu %s, %s, %s\n", current_reg_name, current_reg_name, operand_reg_name);
        } else if(op == '-') {
            fprintf(output_file, "\tdsubu %s, %s, %s\n", current_reg_name, current_reg_name, operand_reg_name);
        } else if(op == '*') {
            fprintf(output_file, "\tdmult %s, %s\n", current_reg_name, operand_reg_name);
            fprintf(output_file, "\tmflo %s\n", current_reg_name);
        } else if(op == '/') {
            fprintf(output_file, "\tddiv %s, %s\n", current_reg_name, operand_reg_name);
            fprintf(output_file, "\tmflo %s\n", current_reg_name);
        }
    }
    
    return current_reg;
}

int syntax_analyzer(const char *source_code, FILE *output_file) {
    int i = 0, line = 1;
    int validation_i = 0;
    int validation_line = 1;
    
    while (source_code[validation_i] != '\0') {
        skip_escape_sequences_and_comments(source_code, &validation_i, &validation_line);
        if(source_code[validation_i] == '\0') break;
        
        int line_start = validation_i;
        int line_num = validation_line;
        int has_content = 0;
        
        while(source_code[validation_i] != '\0' && 
              source_code[validation_i] != '\n' && 
              source_code[validation_i] != ';') {
            if(!isspace((unsigned char)source_code[validation_i])) {
                has_content = 1;
            }
            validation_i++;
        }
        
        if(has_content && source_code[validation_i] != ';') {
            fprintf(stderr, "\nMissing semicolon at line %d\n", line_num);
            fprintf(stderr, "Line content: ");
            
            for(int j = line_start; source_code[j] != '\0' && source_code[j] != '\n'; j++) {
                fputc(source_code[j], stderr);
            }
            fprintf(stderr, "\n");
            exit(1);
        }
        
        if(source_code[validation_i] == ';') {
            validation_i++;
        } else if(source_code[validation_i] == '\n') {
            validation_line++;
            validation_i++;
        }
    }
    
    while (source_code[i] != '\0') {
        skip_escape_sequences_and_comments(source_code, &i, &line);
        if(source_code[i] == '\0') break;

        int start = i;
        
        while (source_code[i] != '\0' && source_code[i] != ';') {
            if(source_code[i] == '\n') line++;
            i++;
        }
        
        if (i > start) {
            int len = i - start;
            
            char *statement = (char *)malloc(len + 2);
            if(statement) {
                strncpy(statement, &source_code[start], len);
                statement[len] = '\0';
                
                white_space_trim(statement);
                
                if (statement[0] != '\0' && statement[0] != ';') {
                    make_assembly_for_statement(output_file, statement);
                }
                
                free(statement);
            }
        }
        
        if (source_code[i] == ';') i++;
    }
    
    return 0;
}

void make_assembly_for_statement(FILE *output_file, const char *statement) {
    if (!statement) return;

    char temp[512];
    strncpy(temp, statement, sizeof(temp)-1);
    temp[sizeof(temp)-1] = '\0';
    white_space_trim(temp);

    if (temp[0] == '\0') return;

    if (strncmp(temp, "int ", 4) == 0) {
        char var_name[64];
        int value = 0;
        int initialized = 0;

        if (sscanf(temp + 4, "%63[^=;] = %d", var_name, &value) == 2) {
            initialized = 1;
            white_space_trim(var_name);
            add_variable(var_name, value, NULL, 0, initialized);
            
            if(output_file) {
                Symbol *sym = find_symbol(var_name);
                if(sym) {
                    const char *var_reg_name = get_register_name(sym->reg_num);
                    const char *r0_name = get_register_name(0);
                    fprintf(output_file, "\tdaddiu %s, %s, %d\n", var_reg_name, r0_name, value);
                    fprintf(output_file, "\tsd %s, %s(%s)\n", var_reg_name, sym->name, r0_name);
                    sym->in_register = 1;
                }
            }
        }
        else if (strchr(temp + 4, '=') != NULL) {
            char rhs[256];
            char *eq_pos = strchr(temp + 4, '=');
            
            int name_len = eq_pos - (temp + 4);
            strncpy(var_name, temp + 4, name_len);
            var_name[name_len] = '\0';
            white_space_trim(var_name);
            
            strcpy(rhs, eq_pos + 1);
            white_space_trim(rhs);
            
            add_variable(var_name, 0, NULL, 0, 0);
            
            if(output_file) {
                char rhs_copy[256];
                strncpy(rhs_copy, rhs, sizeof(rhs_copy)-1);
                rhs_copy[sizeof(rhs_copy)-1] = '\0';
                
                int i = 0;
                while(rhs_copy[i] != '\0') {
                    while(isspace(rhs_copy[i])) i++;
                    if(rhs_copy[i] == '\0') break;
                    
                    if(ALPHABETIC_CHARACTER(rhs_copy[i])) {
                        char rhs_var_name[64];
                        int j = 0;
                        while(ALPHANUMERIC_CHARACTER(rhs_copy[i]) && j < 63) {
                            rhs_var_name[j++] = rhs_copy[i++];
                        }
                        rhs_var_name[j] = '\0';
                        
                        Symbol *rhs_sym = find_symbol(rhs_var_name);
                        if(!rhs_sym) {
                            fprintf(stderr, "\nVariable '%s' used without declaration\n", rhs_var_name);
                            fprintf(stderr, "Statement: %s\n", temp);
                            exit(1);
                        }
                        if(rhs_sym->data_type != 0) {
                            fprintf(stderr, "\nType error: Cannot assign char to int variable '%s'\n", var_name);
                            fprintf(stderr, "Statement: %s\n", temp);
                            exit(1);
                        }
                    } 
                    else if(DIGIT_CHARACTER(rhs_copy[i])) {
                        while(DIGIT_CHARACTER(rhs_copy[i])) i++;
                    } 
                    else if(OPERATOR_CHARACTER(rhs_copy[i])) {
                        i++;
                    } 
                    else {
                        i++;
                    }
                }
                
                Symbol *sym = find_symbol(var_name);
                if(sym) {
                    lexical_analyzer(rhs, output_file, sym->reg_num);
                    
                    const char *var_reg_name = get_register_name(sym->reg_num);
                    const char *r0_name = get_register_name(0);
                    
                    fprintf(output_file, "\tsd %s, %s(%s)\n", var_reg_name, sym->name, r0_name);
                    sym->in_register = 1;
                    sym->initialized = 1;
                }
            }
        }
        else if (sscanf(temp + 4, "%63[^;]", var_name) == 1) {
            white_space_trim(var_name);
            add_variable(var_name, 0, NULL, 0, 0);
            
            if(output_file) {
                Symbol *sym = find_symbol(var_name);
                if(sym) {
                    const char *var_reg_name = get_register_name(sym->reg_num);
                    const char *r0_name = get_register_name(0);
                    fprintf(output_file, "\tdaddiu %s, %s, 0\n", var_reg_name, r0_name);
                    fprintf(output_file, "\tsd %s, %s(%s)\n", var_reg_name, sym->name, r0_name);
                }
            }
        }
        return;
    }

    if (strncmp(temp, "char", 4) == 0) {
        char var_name[64];
        char str_value[256] = "";
        int is_pointer = 0;
        int initialized = 0;
        
        char *ptr_check = temp + 4;
        while(isspace(*ptr_check)) ptr_check++;
        if(*ptr_check == '*') {
            is_pointer = 1;
            ptr_check++;
        }
        
        if(strchr(temp, '=') != NULL && strchr(temp, '"') != NULL) {
            char *eq_pos = strchr(temp, '=');
            
            char *name_start = is_pointer ? ptr_check : (temp + 4);
            while(isspace(*name_start)) name_start++;
            
            int name_len = eq_pos - name_start;
            strncpy(var_name, name_start, name_len);
            var_name[name_len] = '\0';
            white_space_trim(var_name);

            char *bracket = strchr(var_name, '[');
            if(bracket) *bracket = '\0';
            
            char *extracted = extract_string_literal(eq_pos + 1);
            strncpy(str_value, extracted, 255);
            str_value[255] = '\0';
            
            initialized = 1;
            
            int data_type = is_pointer ? 2 : 1;
            add_variable(var_name, 0, str_value, data_type, initialized);
        }
        else {
            char *name_start = is_pointer ? ptr_check : (temp + 4);
            while(isspace(*name_start)) name_start++;
            
            sscanf(name_start, "%63[^;]", var_name);
            white_space_trim(var_name);

            char *bracket = strchr(var_name, '[');
            if(bracket) *bracket = '\0';
            
            int data_type = is_pointer ? 2 : 1;
            add_variable(var_name, 0, "", data_type, 0);
        }
        return;
    }

    char *eq_pos = strchr(temp, '=');
    if(eq_pos != NULL) {
        char lhs[64];
        char rhs[256];
        
        int lhs_len = eq_pos - temp;
        strncpy(lhs, temp, lhs_len);
        lhs[lhs_len] = '\0';
        white_space_trim(lhs);
        
        strcpy(rhs, eq_pos + 1);
        white_space_trim(rhs);
        
        Symbol *sym = find_symbol(lhs);
        if(!sym) {
            fprintf(stderr, "\nVariable '%s' used without declaration\n", lhs);
            fprintf(stderr, "Statement: %s\n", temp);
            exit(1);
        }
        
        if(sym->data_type == 0) {
            char rhs_copy[256];
            strncpy(rhs_copy, rhs, sizeof(rhs_copy)-1);
            rhs_copy[sizeof(rhs_copy)-1] = '\0';
            
            int i = 0;
            while(rhs_copy[i] != '\0') {
                while(isspace(rhs_copy[i])) i++;
                if(rhs_copy[i] == '\0') break;
                
                if(ALPHABETIC_CHARACTER(rhs_copy[i])) {
                    char var_name[64];
                    int j = 0;
                    while(ALPHANUMERIC_CHARACTER(rhs_copy[i]) && j < 63) {
                        var_name[j++] = rhs_copy[i++];
                    }
                    var_name[j] = '\0';
                    
                    Symbol *rhs_sym = find_symbol(var_name);
                    if(!rhs_sym) {
                        fprintf(stderr, "\nVariable '%s' used without declaration\n", var_name);
                        fprintf(stderr, "Statement: %s\n", temp);
                        exit(1);
                    }
                    if(rhs_sym->data_type != 0) {
                        fprintf(stderr, "\nType error: Cannot assign char to int variable '%s'\n", lhs);
                        fprintf(stderr, "Statement: %s\n", temp);
                        exit(1);
                    }
                } 
                else if(DIGIT_CHARACTER(rhs_copy[i])) {
                    while(DIGIT_CHARACTER(rhs_copy[i])) i++;
                } 
                else if(OPERATOR_CHARACTER(rhs_copy[i])) {
                    i++;
                } 
                else {
                    i++;
                }
            }
            
            if(output_file) {
                sym->initialized = 1;
                lexical_analyzer(rhs, output_file, sym->reg_num);
                
                const char *var_reg_name = get_register_name(sym->reg_num);
                const char *r0_name = get_register_name(0);
                
                fprintf(output_file, "\tsd %s, %s(%s)\n", var_reg_name, sym->name, r0_name);
                sym->in_register = 1;
            }
        } 
        else if(sym->data_type == 1 || sym->data_type == 2) {
            if(strchr(rhs, '"') != NULL) {
                char *extracted = extract_string_literal(rhs);
                strncpy(sym->str_value, extracted, 255);
                sym->str_value[255] = '\0';
                sym->initialized = 1;
            } else {
                fprintf(stderr, "\nType error: Expected string literal for char variable '%s'\n", lhs);
                fprintf(stderr, "Statement: %s\n", temp);
                exit(1);
            }
        }
        return;
    }
}