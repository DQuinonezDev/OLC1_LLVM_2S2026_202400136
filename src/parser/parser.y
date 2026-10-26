%code requires {
    #include "ast.h"
}

%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "fuente.h"

int yylex(void);
void yyerror(const char *message);
extern int lexical_error;
int syntax_error = 0;
ASTNode *root = NULL;

/* Convierte la ubicacion de Bison (@n) en la Ubicacion del AST. */
#define U(l) ((Ubicacion){ (l).first_line, (l).first_column, (l).last_line, (l).last_column })
%}

%locations
%define parse.error verbose

%union {
    int ival;
    double dval;
    char *str;
    ASTNode *node;
}

%token LET PRINT LED ON OFF
%token <ival> ENTERO
%token <dval> DECIMAL
%token <str> ID
%type <node> instrucciones instruccion expresion termino factor

%destructor { free($$); } <str>
%destructor { ast_free($$); } <node>

%start programa

/* Sin %left, %right, %nonassoc, %precedence ni %prec:
   - la PRECEDENCIA sale de los niveles expresion > termino > factor
     (lo que esta mas abajo en la gramatica se agrupa primero);
   - la ASOCIATIVIDAD IZQUIERDA sale de la recursion por la izquierda
     (expresion : expresion '+' termino), asi 9 - 3 - 1 = (9 - 3) - 1. */

%%

programa
    : instrucciones                     { root = $1; }
    ;

instrucciones
    : instrucciones instruccion         { $$ = ast_append($1, $2); }
    | instruccion                       { $$ = $1; }
    ;

instruccion
    : LET ID '=' expresion ';'          { $$ = ast_let($2, $4, U(@2)); }
    | PRINT expresion ';'               { $$ = ast_print_stmt($2, U(@1)); }
    | LED expresion ON ';'              { $$ = ast_led($2, 1, U(@1)); }
    | LED expresion OFF ';'             { $$ = ast_led($2, 0, U(@1)); }
    | error ';'                         { $$ = NULL; yyerrok; }
    ;

expresion
    : expresion '+' termino             { $$ = ast_binop('+', $1, $3, U(@$), U(@2)); }
    | expresion '-' termino             { $$ = ast_binop('-', $1, $3, U(@$), U(@2)); }
    | termino                           { $$ = $1; }
    ;

termino
    : termino '*' factor                { $$ = ast_binop('*', $1, $3, U(@$), U(@2)); }
    | termino '/' factor                { $$ = ast_binop('/', $1, $3, U(@$), U(@2)); }
    | factor                            { $$ = $1; }
    ;

factor
    : ENTERO                            { $$ = ast_entero($1, U(@1)); }
    | DECIMAL                           { $$ = ast_decimal($1, U(@1)); }
    | ID                                { $$ = ast_id($1, U(@1)); }
    | '(' expresion ')'                 { $$ = $2; }
    ;

%%

void yyerror(const char *message)
{
    syntax_error = 1;
    fprintf(stderr, "Error sintactico en linea %d, columna %d: %s\n",
            yylloc.first_line, yylloc.first_column, message);
    fuente_subrayar(U(yylloc));
}
