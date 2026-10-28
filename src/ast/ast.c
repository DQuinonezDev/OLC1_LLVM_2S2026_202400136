#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

static ASTNode *ast_new(NodeType type, Ubicacion loc)
{
    ASTNode *node = calloc(1, sizeof *node);
    if (!node) {
        fputs("No hay memoria disponible.\n", stderr);
        exit(EXIT_FAILURE);
    }
    node->type = type;
    node->loc = loc;
    node->tipo = T_DESCONOCIDO;
    return node;
}

ASTNode *ast_let(char *name, ASTNode *expr, Ubicacion loc)
{
    ASTNode *node = ast_new(NODE_LET, loc);
    node->name = name;
    node->left = expr;
    return node;
}

ASTNode *ast_print_stmt(ASTNode *expr, Ubicacion loc)
{
    ASTNode *node = ast_new(NODE_PRINT, loc);
    node->left = expr;
    return node;
}

ASTNode *ast_led(ASTNode *pin, int encendido, Ubicacion loc)
{
    ASTNode *node = ast_new(NODE_LED, loc);
    node->left = pin;
    node->ival = encendido;
    return node;
}

ASTNode *ast_binop(char op, ASTNode *left, ASTNode *right, Ubicacion loc, Ubicacion op_loc)
{
    ASTNode *node = ast_new(NODE_BINOP, loc);
    node->op = op;
    node->op_loc = op_loc;
    node->left = left;
    node->right = right;
    return node;
}

ASTNode *ast_entero(int value, Ubicacion loc)
{
    ASTNode *node = ast_new(NODE_ENTERO, loc);
    node->ival = value;
    return node;
}

ASTNode *ast_decimal(double value, Ubicacion loc)
{
    ASTNode *node = ast_new(NODE_DECIMAL, loc);
    node->dval = value;
    return node;
}

ASTNode *ast_id(char *name, Ubicacion loc)
{
    ASTNode *node = ast_new(NODE_ID, loc);
    node->name = name;
    return node;
}

/* Agrega una instruccion al final de la lista. Los NULL vienen de la
   regla de recuperacion "error ';'" y simplemente se ignoran. */
ASTNode *ast_append(ASTNode *list, ASTNode *node)
{
    if (!node) return list;
    if (!list) return node;
    ASTNode *last = list;
    while (last->next) last = last->next;
    last->next = node;
    return list;
}

const char *tipo_nombre(Tipo tipo)
{
    switch (tipo) {
        case T_INT:    return "i32";
        case T_DOUBLE: return "double";
        case T_ERROR:  return "error";
        default:       return "?";
    }
}

/* Dibuja el arbol con ramas al estilo "tree":
       BINOP(+) : i32
       |-- ENTERO(10) : i32
       `-- BINOP(*) : i32                                       */
static void print_expr(const ASTNode *node, const char *prefix, int last)
{
    if (!node) return;
    printf("%s%s", prefix, last ? "`-- " : "|-- ");
    switch (node->type) {
        case NODE_ENTERO:  printf("ENTERO(%d)", node->ival); break;
        case NODE_DECIMAL:
            printf(node->dval < 1e15 && node->dval == (long long)node->dval
                       ? "DECIMAL(%.1f)" : "DECIMAL(%g)", node->dval);
            break;
        case NODE_ID:      printf("ID(%s)", node->name); break;
        case NODE_BINOP:   printf("BINOP(%c)", node->op); break;
        default: break;
    }
    if (node->tipo != T_DESCONOCIDO)
        printf(" : %s", tipo_nombre(node->tipo));
    putchar('\n');

    if (node->type == NODE_BINOP) {
        char child_prefix[256];
        snprintf(child_prefix, sizeof child_prefix, "%s%s", prefix, last ? "    " : "|   ");
        print_expr(node->left, child_prefix, 0);
        print_expr(node->right, child_prefix, 1);
    }
}

void ast_print(const ASTNode *node)
{
    puts("PROGRAMA");
    for (; node; node = node->next) {
        int last = node->next == NULL;
        printf("%s", last ? "`-- " : "|-- ");
        switch (node->type) {
            case NODE_LET:   printf("LET %s", node->name); break;
            case NODE_PRINT: printf("PRINT"); break;
            case NODE_LED:   printf("LED %s", node->ival ? "on" : "off"); break;
            default: break;
        }
        printf("  (linea %d)\n", node->loc.line);
        print_expr(node->left, last ? "    " : "|   ", 1);
    }
}

static void free_expr(ASTNode *node)
{
    if (!node) return;
    free_expr(node->left);
    free_expr(node->right);
    free(node->name);
    free(node);
}

void ast_free(ASTNode *node)
{
    while (node) {
        ASTNode *next = node->next;
        node->next = NULL;
        free_expr(node);
        node = next;
    }
}
