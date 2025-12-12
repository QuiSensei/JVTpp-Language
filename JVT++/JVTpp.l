%{
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
%}

%option noyywrap
%option yylineno

%%
    [ \t]+      ; // Ignore whitespaces and tabs
    \/\/.*      ; // Single-line comments
    \n          { return NEWLINE; }

    // Keywords:
    develop     { return DEVELOP; }
    show        { return SHOW; }
    finish      { return FINISH; }

    // Operators:
    "+"         { return PLUS;}
    "-"         { return MINUS; }
    "*"         { return MULTIPLICATION; }
    "/"         { return DIVISION; }
    "="         { return ASSIGN }
    "["         { return L_BRACKET; }
    "]"         { return R_BRACKET; }
    "("         { return L_PARENTESIS; }
    ")"         { return R_PARENTESIS; }

    // Punctuation
    ";"         { return SEMICOLON; }
    ","         { return COMMA; }

    // Literals
    [0-9]+ { 
        yylval.num = atoi(yytext);
        return NUMERAL;          // integer type
    }

    \"([^\\\"]|\\.)*\" {
        yylval.str = strdup(yytext);
        return STRING_LATERAL;
    }

    \'([^'\\]|\\.)\'  {
        yylval.str = strdup(yytext);
        return CHARACTER;
    }

    // Identifiers
    [a-zA-Z_][a-zA-Z0-9_]* {  
        yylval.str = strdup(yytext);
        return IDENTIFIER;
    }

    . {
        fprintf(stderr, "ERROR: Unexpected character: '%c' at line %d\n", yytext[0], yylineno);
        exit(1);
    }
    
%%