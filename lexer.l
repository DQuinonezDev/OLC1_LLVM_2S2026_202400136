%option noyywrap yylineno nounput noinput never-interactive nodefault

%{
#include "parser.tab.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int lexical_error = 0;
static int yycolumn = 1;

/* Antes de cada accion se guarda la ubicacion del token (linea y columna)
   para que el parser y el analisis semantico puedan reportar errores. */
#define YY_USER_ACTION                     \
    yylloc.first_line = yylineno;          \
    yylloc.first_column = yycolumn;        \
    yylloc.last_line = yylineno;           \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;

static void lex_error(const char *reason)
{
    fprintf(stderr, "Error lexico en linea %d, columna %d: %s [%s]\n",
            yylloc.first_line, yylloc.first_column, reason, yytext);
    lexical_error = 1;
}

static char *copy_text(const char *text)
{
    size_t length = strlen(text);
    char *copy = malloc(length + 1);
    if (!copy) {
        fputs("No hay memoria disponible.\n", stderr);
        exit(EXIT_FAILURE);
    }
    memcpy(copy, text, length + 1);
    return copy;
}
%}

DIGITO      [0-9]
LETRA       [a-zA-Z_]

%%

"let"       { return LET; }
"print"     { return PRINT; }

{DIGITO}+"."{DIGITO}+ {
    errno = 0;
    double value = strtod(yytext, NULL);
    if (errno == ERANGE)
        lex_error("Decimal fuera del rango de double");
    yylval.dval = value;
    return DECIMAL;
}

{DIGITO}+ {
    errno = 0;
    long value = strtol(yytext, NULL, 10);
    if (errno == ERANGE || value > INT_MAX) {
        lex_error("Entero fuera del rango de i32");
        value = 0;
    }
    yylval.ival = (int)value;
    return ENTERO;
}

{LETRA}({LETRA}|{DIGITO})* {
    yylval.str = copy_text(yytext);
    return ID;
}

"+"         { return '+'; }
"-"         { return '-'; }
"*"         { return '*'; }
"/"         { return '/'; }
"="         { return '='; }
";"         { return ';'; }
"("         { return '('; }
")"         { return ')'; }

"//"[^\r\n]*    { /* Comentario de una linea. */ }
\r?\n           { yycolumn = 1; }
[ \t\r]+        { /* Separadores. */ }
.               { lex_error("Caracter no reconocido"); }

%%
