%{
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    #include "tools.h"

    extern int yylex();
    extern void yyerror(const char *s);
    extern int yylineno;
    extern FILE *yyin;

    int lineCount = 1, cntr = 0; // track real line number per input line
%}

// Define types for semantic values
%union {
    int num;
    char *str;
}

// JVT++ Tokens
%token NEWLINE
%token develop finish show set
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
                    ;

    statements:     statements statement
                    | /* empty */
                    ;

    statement:      declaration SEMICOLON
                    | assignment SEMICOLON
                    | print SEMICOLON
                    | expression SEMICOLON
                    ;
                
    declaration:    set IDENTIFIER ASSIGN expression
                    { 
                        add_symbol($2, $4);
                        free($2);
                    }
                    | set IDENTIFIER ASSIGN STRING
                    {
                        char *clean = strip_quotes($4);
                        add_string_symbol($2, clean);
                        free(clean);
                        free($2);
                        free($4);
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
                    {
                        $$ = strdup("string");
                    }
                    ;

    assignment:     IDENTIFIER ASSIGN expression
                    { 
                        add_symbol($1, $3);
                        free($1);
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
                    | show L_PARENTESIS STRING COMMA IDENTIFIER R_PARENTESIS 
                    {
                        char *remove_q = strip_quotes($3);
                        int idx = lookup_symbol($5);
                        if (idx != -1 && !symbol_table[idx].is_string) {
                            printf(remove_q, symbol_table[idx].value);
                        } else {
                            printf(remove_q, $5);
                        }
                        free(remove_q);
                        free($3);
                        free($5);
                    }
                    | show L_PARENTESIS STRING R_PARENTESIS 
                    {
                        char *text = strip_quotes($3);
                        printf("%s", text);
                        free(text);
                        free($3);
                    }
                    | show L_PARENTESIS CHARACTER R_PARENTESIS 
                    {
                        char *text = strip_quotes($3);
                        printf("%s", text);
                        free(text);
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
                            yyerror("Division by zero");
                            $$ = 0;
                        } else {
                            $$ = $1 / $3;
                        }
                    }
                    | factor
                    { $$ = $1; }
                    ;

    factor:         IDENTIFIER
                    { 
                        $$ = get_symbol_value($1);  // FIXED: Added $$
                        free($1);
                    }
                    | NUMERAL
                    { $$ = $1; }
                    | L_PARENTESIS expression R_PARENTESIS
                    { $$ = $2; }
                    ;
%%

void yyerror(const char *s) {
    fprintf(stderr, "Error at line %d: %s\n", yylineno, s);
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
