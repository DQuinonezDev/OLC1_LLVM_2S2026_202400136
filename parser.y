%code requires {
    #include "ast.h"
}

%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

int yylex(void);
void yyerror(const char *message);
extern int lexical_error;
int syntax_error = 0;
ASTNode *root = NULL;
%}

%locations
%define parse.error verbose

%union {
    int ival;
    double dval;
    char *str;
    ASTNode *node;
}

%token LET PRINT
%token <ival> ENTERO
%token <dval> DECIMAL
%token <str> ID
%type <node> instrucciones instruccion expresion termino factor

%destructor { free($$); } <str>
%destructor { ast_free($$); } <node>

%start programa

/* Sin %left, %right, %nonassoc, %precedence ni %prec:
   la precedencia sale de los niveles expresion > termino > factor
   y la asociatividad izquierda de la recursion por la izquierda. */

%%

programa
    : instrucciones                     { root = $1; }
    ;

instrucciones
    : instrucciones instruccion         { $$ = ast_append($1, $2); }
    | instruccion                       { $$ = $1; }
    ;

instruccion
    : LET ID '=' expresion ';'          { $$ = ast_let($2, $4, @2.first_line, @2.first_column); }
    | PRINT expresion ';'               { $$ = ast_print_stmt($2, @1.first_line, @1.first_column); }
    | error ';'                         { $$ = NULL; yyerrok; }
    ;

expresion
    : expresion '+' termino             { $$ = ast_binop('+', $1, $3, @1.first_line, @1.first_column); }
    | expresion '-' termino             { $$ = ast_binop('-', $1, $3, @1.first_line, @1.first_column); }
    | termino                           { $$ = $1; }
    ;

termino
    : termino '*' factor                { $$ = ast_binop('*', $1, $3, @1.first_line, @1.first_column); }
    | termino '/' factor                { $$ = ast_binop('/', $1, $3, @1.first_line, @1.first_column); }
    | factor                            { $$ = $1; }
    ;

factor
    : ENTERO                            { $$ = ast_entero($1, @1.first_line, @1.first_column); }
    | DECIMAL                           { $$ = ast_decimal($1, @1.first_line, @1.first_column); }
    | ID                                { $$ = ast_id($1, @1.first_line, @1.first_column); }
    | '(' expresion ')'                 { $$ = $2; }
    ;

%%

void yyerror(const char *message)
{
    syntax_error = 1;
    fprintf(stderr, "Error sintactico en linea %d, columna %d: %s\n",
            yylloc.first_line, yylloc.first_column, message);
}
