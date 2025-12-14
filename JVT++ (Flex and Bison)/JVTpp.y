%{
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    #include "tools.h"

    extern int yylex();
    extern void yyerror(const char *s);
    extern int yylineno;
    extern char *current_format_string;
    extern FILE *yyin;

    int lineCount = 1, cntr = 2;
%}

%error-verbose

//
%union {
    int num;
    char *str;
}

// JVT++ Tokens
%token develop finish show showln
%token NUMERAL ALPHA
%token PLUS MINUS MULTIPLICATION DIVISION ASSIGN
%token L_BRACKET R_BRACKET L_PARENTESIS R_PARENTESIS
%token SEMICOLON COMMA
%token <num> INTEGER
%token <str> STRING
%token <str> CHARACTER
%token <str> IDENTIFIER

// Operator precedence
%left PLUS MINUS
%left MULTIPLICATION DIVISION
%right ASSIGN

%type <num> expression term factor
%type <str> data_type

// JVT++ CFG
%%
program:        develop L_BRACKET statements R_BRACKET finish
                | error L_BRACKET statements R_BRACKET finish
                {
                    yyerror("Missing or misspelled 'develop' keyword");
                    yyerrok;
                }
                | develop error statements R_BRACKET finish
                {
                    yyerror("Missing '[' after 'develop'");
                    yyerrok;
                }
                | develop L_BRACKET statements error finish
                {
                    yyerror("Missing ']' before 'finish'");
                    yyerrok;
                }       
                | develop L_BRACKET statements R_BRACKET error
                {
                    yyerror("Missing or misspelled 'finish' keyword");
                    yyerrok;
                }
                ;

statements:     statements statement
                | /* empty */
                ;

statement:      declaration SEMICOLON
                | declaration error
                {
                    yyerror("expected ';' after declaration");
                    yyerrok;
                }
                | assignment SEMICOLON
                | assignment error
                {
                    yyerror("expected ';' after assignment");
                    yyerrok;
                }
                | print SEMICOLON
                | print error
                {
                    yyerror("expected ';' after show");
                    yyerrok;
                }
                | println SEMICOLON
                | println error
                {
                    yyerror("expected ';' after show");
                    yyerrok;
                }
                ;
            
declaration:    data_type IDENTIFIER
                {
                    if (strcmp($1, "int") == 0) {
                        add_symbol($2, 0);          // default int = 0
                    } else {
                        add_string_symbol($2, "");  // default string = ""
                    }
                    free($1);
                    free($2);
                }
                | data_type IDENTIFIER ASSIGN expression
                {
                    add_symbol($2, $4);
                    free($1);
                    free($2);
                }
                | data_type IDENTIFIER ASSIGN STRING
                {
                    char *clean = strip_quotes($4);
                    add_string_symbol($2, clean);
                    free(clean);
                    free($1);
                    free($2);
                    free($4);
                }
                ;

data_type:      NUMERAL
                { $$ = strdup("int"); }
                | ALPHA
                { $$ = strdup("string"); }
                ;

assignment:     IDENTIFIER ASSIGN expression
                { 
                    add_symbol($1, $3);
                    free($1);
                }
                | IDENTIFIER ASSIGN STRING
                {
                    char *clean = strip_quotes($3);
                    add_string_symbol($1, clean);
                    free(clean);
                    free($1);
                    free($3);
                }
                ;

print:          show L_PARENTESIS IDENTIFIER R_PARENTESIS 
                { 
                    int idx = lookup_symbol($3);
                    if (idx != -1) {
                        if (symbol_table[idx].is_string) {
                            printf("%s", symbol_table[idx].str_value);
                        } else {
                            printf("%d", symbol_table[idx].value);
                        }
                    } else {
                        printf("%s", $3);
                    }
                    free($3);
                }
                | show L_PARENTESIS STRING R_PARENTESIS 
                {
                    if (strchr($3, '{')) {
                        print_fstring($3);
                    } else {
                        char *text = strip_quotes($3);
                        printf("%s", text);
                        free(text);
                    }
                    free($3);
                }
                | show L_PARENTESIS CHARACTER R_PARENTESIS 
                {
                    char *text = strip_quotes($3);
                    printf("%s", text);
                    free(text);
                    free($3);
                }
                | show L_PARENTESIS STRING COMMA expression R_PARENTESIS 
                {
                    char *remove_q = strip_quotes($3);
                    printf(remove_q, $5);
                    free(remove_q);
                    free($3);
                }
                | show L_PARENTESIS STRING COMMA expression COMMA expression R_PARENTESIS 
                {
                    char *remove_q = strip_quotes($3);
                    printf(remove_q, $5, $7);
                    free(remove_q);
                    free($3);
                }
                | show L_PARENTESIS STRING COMMA expression COMMA expression COMMA expression R_PARENTESIS 
                {
                    char *remove_q = strip_quotes($3);
                    printf(remove_q, $5, $7, $9);
                    free(remove_q);
                    free($3);
                }
                | show L_PARENTESIS STRING COMMA expression COMMA expression COMMA expression COMMA expression R_PARENTESIS 
                {
                    char *remove_q = strip_quotes($3);
                    printf(remove_q, $5, $7, $9, $11);
                    free(remove_q);
                    free($3);
                }
                | show L_PARENTESIS STRING COMMA CHARACTER R_PARENTESIS 
                {
                    char *fmt = strip_quotes($3);
                    char *ch = strip_quotes($5);
                    printf(fmt, ch[0]);
                    free(fmt);
                    free(ch);
                    free($3);
                    free($5);
                }
                ;

println:        showln L_PARENTESIS IDENTIFIER R_PARENTESIS 
                { 
                    int idx = lookup_symbol($3);
                    if (idx != -1) {
                        if (symbol_table[idx].is_string) {
                            printf("%s\n", symbol_table[idx].str_value);
                        } else {
                            printf("%d\n", symbol_table[idx].value);
                        }
                    } else {
                        printf("%s\n", $3);
                    }
                    free($3);
                }
                | showln L_PARENTESIS STRING R_PARENTESIS 
                {
                    if (strchr($3, '{')) {
                        println_fstring($3);
                    } else {
                        char *text = strip_quotes($3);
                        printf("%s\n", text);
                        free(text);
                    }
                    free($3);
                }
                | showln L_PARENTESIS CHARACTER R_PARENTESIS 
                {
                    char *text = strip_quotes($3);
                    printf("%s\n", text);
                    free(text);
                    free($3);
                }
                | showln L_PARENTESIS STRING COMMA expression R_PARENTESIS 
                {
                    char *remove_q = strip_quotes($3);
                    current_format_string = remove_q;  // Set context
                    printf(remove_q, $5);
                    current_format_string = NULL;  // Clear context
                    printf("\n");
                    free(remove_q);
                    free($3);
                }
                | showln L_PARENTESIS STRING COMMA expression COMMA expression R_PARENTESIS 
                {
                    char *remove_q = strip_quotes($3);
                    current_format_string = remove_q;  // Set context
                    printf(remove_q, $5, $7);
                    current_format_string = NULL;  // Clear context
                    printf("\n");
                    free(remove_q);
                    free($3);
                }
                | showln L_PARENTESIS STRING COMMA expression COMMA expression COMMA expression R_PARENTESIS 
                {
                    char *remove_q = strip_quotes($3);
                    current_format_string = remove_q;  // Set context
                    printf(remove_q, $5, $7, $9);
                    current_format_string = NULL;  // Clear context
                    printf("\n");
                    free(remove_q);
                    free($3);
                }
                | showln L_PARENTESIS STRING COMMA expression COMMA expression COMMA expression COMMA expression R_PARENTESIS 
                {
                    char *remove_q = strip_quotes($3);
                    current_format_string = remove_q;  // Set context
                    printf(remove_q, $5, $7, $9, $11);
                    current_format_string = NULL;  // Clear context
                    printf("\n");
                    free(remove_q);
                    free($3);
                }
                | showln L_PARENTESIS STRING COMMA CHARACTER R_PARENTESIS 
                {
                    char *fmt = strip_quotes($3);
                    char *ch = strip_quotes($5);
                    printf(fmt, ch[0]);
                    printf("\n");
                    free(fmt);
                    free(ch);
                    free($3);
                    free($5);
                }
                ;

expression:     expression PLUS term
                { $$ = $1 + $3; }
                | expression MINUS term
                { $$ = $1 - $3; }
                | term
                { $$ = $1; }
                ;

term:           term MULTIPLICATION factor
                { $$ = $1 * $3; }
                | term DIVISION factor
                {
                    if (division_check($1, $3) == -1) {
                        yyerror("Can't divide by 0");
                        $$ = 0;
                        exit(1);
                    } else {
                        $$ = $1 / $3;
                    }
                }
                | factor
                { $$ = $1; }
                ;

factor:         IDENTIFIER
                { 
                    // Check if it's a string variable being used in expression context
                    int idx = lookup_symbol($1);
                    if (idx != -1 && symbol_table[idx].is_string) {
                        // String in arithmetic expression - cast pointer for printf
                        $$ = (int)(long)symbol_table[idx].str_value;
                    } else {
                        $$ = get_symbol_value($1);
                    }
                    free($1);
                }
                | INTEGER
                { $$ = $1; }
                | L_PARENTESIS expression R_PARENTESIS
                { $$ = $2; }
                ;
%%

void yyerror(const char *s) {
    // Suppress the automatic error
    if (strcmp(s, "syntax error") == 0 || strstr(s, "syntax error") != NULL) return;
    fprintf(stderr, "Error: Line %d %s\n", yylineno, s);
    exit(1);
}

int main(int argc, char **argv) 
{
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "Cannot open file: %s\n", argv[1]);
            return 1;
        }
    }

    int result = yyparse();

    if (argc > 1 && yyin != NULL) {
        fclose(yyin);
    }

    return result;
}
