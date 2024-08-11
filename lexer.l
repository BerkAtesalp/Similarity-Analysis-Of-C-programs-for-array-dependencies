%{
#include "ast.h"
#include "y.tab.h"

extern int yylex(void);
extern int yylineno;
int yywrap(void) {
    return 1;
}
%}

%%
"int"                   { yylval.sval = strdup(yytext); printf("int\n"); return INT; }
"return"                { printf("return\n"); return RETURN; }
"main"                  { printf("main\n"); return MAIN; }
"if"                    { printf("if\n"); return IF; }
"else"                  { printf("else\n"); return ELSE; }
"while"                 { printf("while\n"); return WHILE; }
"printf"                { printf("printf\n"); return PRINTF; }
"for"                   { printf("for\n"); return FOR; }
"void"                  { printf("void\n"); return VOID; }
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.sval = strdup(yytext); printf("%s\n", yytext); return IDENTIFIER; }
[0-9]+                  { yylval.sval = strdup(yytext); printf("%s\n", yytext); return NUMBER; }
"+"                     { printf("+\n"); return PLUS; }
"-"                     { printf("-\n"); return MINUS; }
"*"                     { printf("*\n"); return TIMES; }
"/"                     { printf("/\n"); return DIVIDE; }
"="                     { printf("=\n"); return ASSIGN; }
";"                     { printf(";\n"); return SEMICOLON; }
"("                     { printf("(\n"); return LPAREN; }
")"                     { printf(")\n"); return RPAREN; }
"{"                     { printf("{\n"); return LBRACE; }
"}"                     { printf("}\n"); return RBRACE; }
","                     { printf(",\n"); return COMMA; }
"["                     { printf("[\n"); return LBRACKET; }
"]"                     { printf("]\n"); return RBRACKET; }
"<"                     { printf("<\n"); return LT; }
">"                     { printf(">\n"); return GT; }
"<="                    { printf("<=\n"); return LE; }
">="                    { printf(">=\n"); return GE; }
"=="                    { printf("==\n"); return EQ; }
"!="                    { printf("!=\n"); return NE; }
"++" 					{ printf("++\n"); return PLUSPLUS; }
"--" 					{ printf("--\n"); return MINUSMINUS; }
"&&"                    { printf("&&\n"); return AND; }
"||"                    { printf("||\n"); return OR; }
"!"                     { printf("!\n"); return NOT; }
"%"                     { printf("%%\n"); return MOD; }
"&"                     { printf("&\n"); return BITAND; }
"|"                     { printf("|\n"); return BITOR; }
"^"                     { printf("^\n"); return XOR; }
"<<"                    { printf("<<\n"); return SHL; }
">>"                    { printf(">>\n"); return SHR; }
\"([^"\\]|\\.)*\"       { yylval.sval = strdup(yytext); printf("Token STRING_LITERAL generated: %s\n", yytext); return STRING_LITERAL; }
\n                      { yylineno++; }
\/\/[^\n]*              { /* ignore C++ style comments */ }
\/\*[^*]*\*+(?:[^/*][^*]*\*+)*\/  { /* ignore C style comments */ }
\#.*                    { /* ignore preprocessor directives like #include */ } 
"extern"[ \t]+[^;\n]*;  { /* ignore external declarations */ }
"typedef"[ \t]+[^;\n]*; { /* ignore typedef declarations */ }
"void"[ \t]+[^;\n]*;    { /* ignore void declarations */ }
[ \t\n]+                { /* ignore whitespace */ }
.                       { fprintf(stderr, "Error: Unknown character '%c' at line %d\n", *yytext, yylineno); }

%%
