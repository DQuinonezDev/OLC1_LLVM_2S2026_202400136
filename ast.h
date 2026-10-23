#ifndef AST_H
#define AST_H

/* Tipos del lenguaje. T_DESCONOCIDO es el valor inicial de cada nodo;
   el analisis semantico lo reemplaza por T_INT, T_DOUBLE o T_ERROR. */
typedef enum {
    T_DESCONOCIDO, T_INT, T_DOUBLE, T_ERROR
} Tipo;

typedef enum {
    NODE_LET, NODE_PRINT,
    NODE_ENTERO, NODE_DECIMAL, NODE_ID, NODE_BINOP
} NodeType;

typedef struct ASTNode {
    NodeType type;
    int line, col;          /* ubicacion en el archivo fuente */
    char op;                /* NODE_BINOP: '+', '-', '*', '/' */
    int ival;               /* NODE_ENTERO */
    double dval;            /* NODE_DECIMAL */
    char *name;             /* NODE_ID y NODE_LET */
    struct ASTNode *left;   /* NODE_BINOP: operando izquierdo; LET/PRINT: expresion */
    struct ASTNode *right;  /* NODE_BINOP: operando derecho */
    Tipo tipo;              /* lo llena el analisis semantico */
    struct ASTNode *next;   /* siguiente instruccion del programa */
} ASTNode;

ASTNode *ast_let(char *name, ASTNode *expr, int line, int col);
ASTNode *ast_print_stmt(ASTNode *expr, int line, int col);
ASTNode *ast_binop(char op, ASTNode *left, ASTNode *right, int line, int col);
ASTNode *ast_entero(int value, int line, int col);
ASTNode *ast_decimal(double value, int line, int col);
ASTNode *ast_id(char *name, int line, int col);

ASTNode *ast_append(ASTNode *list, ASTNode *node);
void ast_print(const ASTNode *node, int depth);
void ast_free(ASTNode *node);
const char *tipo_nombre(Tipo tipo);

#endif
