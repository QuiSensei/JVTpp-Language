%{
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    #include "tools.h"
    #include "JVTpp.tab.h"

%}

%option noyywrap
%option yylineno

%%
[ \t]+          ; // Ignore whitespaces and tabs
\/\/.*          ; // Single-line comments
\n              { return NEWLINE; }

develop         { return develop; }
show            { return show; }
finish          { return finish; }
set             { return set; }
NUMERAL         { return NUMERAL; }     // Type keyword for numbers
ALPHA           { return ALPHA; }       // Type keyword for strings

"+"             { return PLUS;}
"-"             { return MINUS; }
"*"             { return MULTIPLICATION; }
"/"             { return DIVISION; }
"="             { return ASSIGN; }
"["             { return L_BRACKET; }
"]"             { return R_BRACKET; }
"("             { return L_PARENTESIS; }
")"             { return R_PARENTESIS; }

";"             { return SEMICOLON; }
","             { return COMMA; }

[0-9]+ { 
    yylval.num = atoi(yytext);
    return INTEGER;          // integer type
}

\"([^\\\"]|\\.)*\" {
    yylval.str = strdup(yytext);
    return STRING;
}

\'([^'\\]|\\.)\'  {
    yylval.str = strdup(yytext);
    return CHARACTER;
}

[a-zA-Z_][a-zA-Z0-9_]* {  
    yylval.str = strdup(yytext);
    return IDENTIFIER;
}

. {
    fprintf(stderr, "ERROR: Unexpected character: '%c' at line %d\n", yytext[0], yylineno);
    exit(1);
}
%%