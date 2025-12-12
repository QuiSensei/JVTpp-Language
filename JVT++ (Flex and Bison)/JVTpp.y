%{
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>

    extern int yylex();
    extern void yyerror(const char *s);
    extern int yylineno;

    int lineCount = 1, cntr = 0; // track real line number per input line


%}

// JVT++ Tokens
%token NEWLINE
%token DEVELOP SHOW FINISH
%token PLUS MINUS MULTIPLICATION DIVISION ASSIGN
%token L_BRACKET R_BRACKET L_PARENTESIS R_PARENTESIS
%token SEMICOLON COMMA
%token <num> NUMERAL
%token <str> STRING_LATERAL
%token <str> CHARACTER
%token <str> IDENTIFIER

