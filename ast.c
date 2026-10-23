#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

static ASTNode *ast_new(NodeType type, int line, int col)
{
    ASTNode *node = calloc(1, sizeof *node);
    if (!node) {
        fputs("No hay memoria disponible.\n", stderr);
        exit(EXIT_FAILURE);
    }
    node->type = type;
    node->line = line;
    node->col = col;
    node->tipo = T_DESCONOCIDO;
    return node;
}

ASTNode *ast_let(char *name, ASTNode *expr, int line, int col)
{
    ASTNode *node = ast_new(NODE_LET, line, col);
    node->name = name;
    node->left = expr;
    return node;
}

ASTNode *ast_print_stmt(ASTNode *expr, int line, int col)
{
    ASTNode *node = ast_new(NODE_PRINT, line, col);
    node->left = expr;
    return node;
}

ASTNode *ast_binop(char op, ASTNode *left, ASTNode *right, int line, int col)
{
    ASTNode *node = ast_new(NODE_BINOP, line, col);
    node->op = op;
    node->left = left;
    node->right = right;
    return node;
}

ASTNode *ast_entero(int value, int line, int col)
{
    ASTNode *node = ast_new(NODE_ENTERO, line, col);
    node->ival = value;
    return node;
}

ASTNode *ast_decimal(double value, int line, int col)
{
    ASTNode *node = ast_new(NODE_DECIMAL, line, col);
    node->dval = value;
    return node;
}

ASTNode *ast_id(char *name, int line, int col)
{
    ASTNode *node = ast_new(NODE_ID, line, col);
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

static void print_expr(const ASTNode *node, int depth)
{
    if (!node) return;
    printf("%*s", depth * 2, "");
    switch (node->type) {
        case NODE_ENTERO:  printf("ENTERO(%d)", node->ival); break;
        case NODE_DECIMAL: printf("DECIMAL(%g)", node->dval); break;
        case NODE_ID:      printf("ID(%s)", node->name); break;
        case NODE_BINOP:   printf("BINOP(%c)", node->op); break;
        default: break;
    }
    if (node->tipo != T_DESCONOCIDO)
        printf(" : %s", tipo_nombre(node->tipo));
    putchar('\n');
    if (node->type == NODE_BINOP) {
        print_expr(node->left, depth + 1);
        print_expr(node->right, depth + 1);
    }
}

void ast_print(const ASTNode *node, int depth)
{
    printf("%*sPROGRAMA\n", depth * 2, "");
    for (; node; node = node->next) {
        printf("%*s", (depth + 1) * 2, "");
        if (node->type == NODE_LET)
            printf("LET %s  (linea %d)\n", node->name, node->line);
        else
            printf("PRINT  (linea %d)\n", node->line);
        print_expr(node->left, depth + 2);
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
