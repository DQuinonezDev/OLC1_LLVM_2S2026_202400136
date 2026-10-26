#ifndef AST_H
#define AST_H

/* Tipos del lenguaje. T_DESCONOCIDO es el valor inicial de cada nodo;
   el analisis semantico lo reemplaza por T_INT, T_DOUBLE o T_ERROR. */
typedef enum {
    T_DESCONOCIDO, T_INT, T_DOUBLE, T_ERROR
} Tipo;

typedef enum {
    /* Instrucciones */
    NODE_LET, NODE_PRINT, NODE_LED,
    /* Expresiones */
    NODE_ENTERO, NODE_DECIMAL, NODE_ID, NODE_BINOP
} NodeType;

/* Region del archivo fuente que ocupa un nodo (para reportar errores). */
typedef struct {
    int line, col;          /* primer caracter */
    int end_line, end_col;  /* ultimo caracter */
} Ubicacion;

typedef struct ASTNode {
    NodeType type;
    Ubicacion loc;          /* region completa del nodo */
    Ubicacion op_loc;       /* NODE_BINOP: posicion del operador */
    char op;                /* NODE_BINOP: '+', '-', '*', '/' */
    int ival;               /* NODE_ENTERO; NODE_LED: 1 = on, 0 = off */
    double dval;            /* NODE_DECIMAL */
    char *name;             /* NODE_ID y NODE_LET */
    struct ASTNode *left;   /* NODE_BINOP: operando izquierdo; LET/PRINT/LED: expresion */
    struct ASTNode *right;  /* NODE_BINOP: operando derecho */
    Tipo tipo;              /* lo llena el analisis semantico */
    struct ASTNode *next;   /* siguiente instruccion del programa */
} ASTNode;

ASTNode *ast_let(char *name, ASTNode *expr, Ubicacion loc);
ASTNode *ast_print_stmt(ASTNode *expr, Ubicacion loc);
ASTNode *ast_led(ASTNode *pin, int encendido, Ubicacion loc);
ASTNode *ast_binop(char op, ASTNode *left, ASTNode *right, Ubicacion loc, Ubicacion op_loc);
ASTNode *ast_entero(int value, Ubicacion loc);
ASTNode *ast_decimal(double value, Ubicacion loc);
ASTNode *ast_id(char *name, Ubicacion loc);

ASTNode *ast_append(ASTNode *list, ASTNode *node);
void ast_print(const ASTNode *node);
void ast_free(ASTNode *node);
const char *tipo_nombre(Tipo tipo);

#endif
