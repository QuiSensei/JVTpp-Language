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
%token develop finish show
%token PLUS MINUS MULTIPLICATION DIVISION ASSIGN
%token L_BRACKET R_BRACKET L_PARENTESIS R_PARENTESIS
%token SEMICOLON COMMA
%token <num> NUMERAL
%token <str> ALPHA
%token <str> CHARACTER
%token <str> IDENTIFIER

// Operator precedence
%left PLUS MINUS
%left MULTIPLICATION DIVISION
%right ASSIGN

%type <num> expression term factor

%expect 1

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
                    ;
    
    set:            NUMERAL
                    | ALPHA
                    ;

    assignment:     IDENTIFIER ASSIGN expression
                    ;

    // From hard copy CFG
    // print:          SHOW L_PARENTESIS IDENTIFIER R_PARENTESIS
    //                 | SHOW L_PARENTESIS CHAR_LIT COMMA expression R_PARENTESIS
    //                 | SHOW L_PARENTESIS CHAR_LIT COMMA IDENTIFIER R_PARENTESIS
    //                 | SHOW L_PARENTESIS CHAR_LIT R_PARENTESIS
    //                 ;

    // Fixxed version  
    print:          show L_PARENTESIS IDENTIFIER R_PARENTESIS 
                    { 
                        printf("%s", $3); 
                    }
                    | show L_PARENTESIS ALPHA COMMA expression R_PARENTESIS 
                    {
                        char *remove_q = strip_quotes($3);
                        printf(remove_q, $5);
                        free(remove_q);
                        free($3);
                    }
                    | show L_PARENTESIS ALPHA COMMA expression COMMA expression R_PARENTESIS 
                    {
                        char *remove_q = strip_quotes($3);
                        printf(remove_q, $5, $7);
                        free(remove_q);
                        free($3);
                    }
                    | show L_PARENTESIS ALPHA COMMA expression COMMA expression COMMA expression R_PARENTESIS 
                    {
                        char *remove_q = strip_quotes($3);
                        printf(remove_q, $5, $7, $9);
                        free(remove_q);
                        free($3);
                    }
                    | show L_PARENTESIS ALPHA COMMA IDENTIFIER R_PARENTESIS 
                    {
                        char *remove_q = strip_quotes($3);
                        printf(remove_q, $5);
                        free(remove_q);
                        free($3);
                        free($5);
                    }
                    | show L_PARENTESIS ALPHA R_PARENTESIS 
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
                    ;

    expression:     expression PLUS term
                    { $$ = $1 + $3 }
                    | expression MINUS term
                    { $$ = $1 - $3; }
                    | term
                    { $$ = $1; }
                    ;

    term:           term MULTIPLICATION factor
                    { $$ = $1 * $3; }
                    | term DIVISION factor
                    {
                        if(division_check($1, $3) == -1) {
                            yyerror("Division by zero");
                            $$ = 0;
                        }
                    }
                    | factor
                    { $$ = $1; }
                    ;

    factor:         IDENTIFIER
                    { $$ = 0; }
                    | NUMERAL
                    { $$ = $1; }
                    | L_PARENTESIS expression R_PARENTESIS
                    { $$ = $2; }
                    ;
%%

int main(int argc, char **argv) 
{
    printf("JVT++ Parser\n");
    printf("============\n\n");

    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "Cannot open file: %s\n", argv[1]);
            return 1;
        }
    }

    int result = yyparse();

    if (result == 0) {
        printf("\nParsing completed successfully.\n");
    } else {
        printf("\nParsing failed with errors.\n");
    }

    if (argc > 1 && yyin != NULL) {
        fclose(yyin);
    }

    return result;
}

void yyerror(const char *s) {
    fprintf(stderr, "Error at line %d: %s\n", yylineno, s);
}

