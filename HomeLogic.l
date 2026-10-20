%{
#include "HomeLogic.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
%}

%option noyywrap
%option yylineno

%%

"DEVICE"        { return DEVICE; }
"SENSOR"        { return SENSOR; }
"VAR"           { return VAR; }
"SWITCHON"      { return SWITCHON; }
"SWITCHOFF"     { return SWITCHOFF; }
"DIM"           { return DIM; }
"SENSE"         { return SENSE; }
"INPUT"         { return INPUT; }
"SHOW"          { return SHOW; }
"WHEN"          { return WHEN; }
"OTHERWISE"     { return OTHERWISE; }
"REPEAT"        { return REPEAT; }
"ROUTINE"       { return ROUTINE; }
"RUN"           { return RUN; }
"AT"            { return AT; }
"CLOCK"         { return CLOCK; }
"AND"           { return AND; }
"OR"            { return OR; }
"NOT"           { return NOT; }

[0-9]{1,2}":"[0-9]{2}   {
                    int h, m;
                    sscanf(yytext, "%d:%d", &h, &m);
                    yylval.num = h * 100 + m;
                    return TIME;
                }

[0-9]+          { yylval.num = atoi(yytext); return NUMBER; }

[a-zA-Z_][a-zA-Z0-9_]*   { yylval.str = strdup(yytext); return ID; }

\"[^\"\n]*\"    {
                    size_t n = yyleng - 2;
                    char *txt = (char *)malloc(n + 1);
                    memcpy(txt, yytext + 1, n);
                    txt[n] = 0;
                    yylval.str = txt;
                    return STRING;
                }

"//"[^\n]*     { /* line comment */ }
[ \t\r\n]+      { /* ignore whitespace */ }

"=="            { return EQ; }
"!="            { return NE; }
"<="            { return LE; }
">="            { return GE; }
"<"             { return '<'; }
">"             { return '>'; }
"="             { return '='; }
";"             { return ';'; }
"+"             { return '+'; }
"-"             { return '-'; }
"*"             { return '*'; }
"/"             { return '/'; }
"("             { return '('; }
")"             { return ')'; }
"{"             { return '{'; }
"}"             { return '}'; }

.               { printf("HomeLogic lexical error, line %d: stray character '%s'\n", yylineno, yytext); }

%%
