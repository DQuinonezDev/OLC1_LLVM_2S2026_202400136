#include "semantic.h"
#include "fuente.h"
#include <stdio.h>

static int error_count = 0;

/* Formato inspirado en la Figura 10 del enunciado: ubicacion, simbolo o
   tipos involucrados, causa y la linea fuente subrayada. */
static void semantic_error(Ubicacion at, Ubicacion underline, const char *titulo,
                           const char *detalle_nombre, const char *detalle,
                           const char *causa)
{
    error_count++;
    fprintf(stderr, "Error semantico en linea %d, columna %d: %s\n", at.line, at.col, titulo);
    if (detalle)
        fprintf(stderr, "  %-8s %s\n", detalle_nombre, detalle);
    fprintf(stderr, "  %-8s %s\n", "Causa", causa);
    fuente_subrayar(underline);
}

/* Devuelve el tipo de la expresion y lo guarda en node->tipo.
   T_ERROR significa "ya se reporto un error dentro de esta expresion";
   se propaga hacia arriba sin volver a reportar para no generar una
   cascada de mensajes por un solo problema. */
static Tipo check_expr(ASTNode *node, TablaSimbolos *tabla)
{
    switch (node->type) {
        case NODE_ENTERO:
            node->tipo = T_INT;
            break;

        case NODE_DECIMAL:
            node->tipo = T_DOUBLE;
            break;

        case NODE_ID: {
            Simbolo *s = tabla_buscar(tabla, node->name);
            if (!s) {
                semantic_error(node->loc, node->loc, "variable no declarada",
                               "Simbolo", node->name,
                               "la variable se usa sin una declaracion 'let' previa");
                node->tipo = T_ERROR;
            } else {
                node->tipo = s->tipo;
            }
            break;
        }

        case NODE_BINOP: {
            Tipo l = check_expr(node->left, tabla);
            Tipo r = check_expr(node->right, tabla);

            if (l == T_ERROR || r == T_ERROR) {
                node->tipo = T_ERROR;
            } else if (l != r) {
                char detalle[64];
                snprintf(detalle, sizeof detalle, "%s %c %s", tipo_nombre(l), node->op, tipo_nombre(r));
                semantic_error(node->op_loc, node->loc, "operacion entre tipos incompatibles",
                               "Tipos", detalle,
                               "el lenguaje no hace conversion implicita entre entero y decimal");
                node->tipo = T_ERROR;
            } else if (l == T_INT && node->op == '/' &&
                       node->right->type == NODE_ENTERO && node->right->ival == 0) {
                /* sdiv entre 0 es comportamiento indefinido en LLVM. */
                semantic_error(node->op_loc, node->loc, "division entera entre cero",
                               "Tipos", "i32 / 0",
                               "sdiv con divisor 0 no tiene resultado definido");
                node->tipo = T_ERROR;
            } else {
                node->tipo = l;
            }
            break;
        }

        default:
            node->tipo = T_ERROR;
            break;
    }
    return node->tipo;
}

int semantic_analyze(ASTNode *root, TablaSimbolos *tabla)
{
    error_count = 0;

    for (ASTNode *stmt = root; stmt; stmt = stmt->next) {
        switch (stmt->type) {
            case NODE_LET: {
                /* Primero la expresion: en "let x = x + 1;" la x de la
                   derecha todavia no existe y debe reportarse. */
                Tipo t = check_expr(stmt->left, tabla);
                Simbolo *previo = tabla_buscar(tabla, stmt->name);

                if (previo) {
                    char detalle[160];
                    snprintf(detalle, sizeof detalle, "%s (ya declarada en linea %d como %s)",
                             stmt->name, previo->decl.line, tipo_nombre(previo->tipo));
                    semantic_error(stmt->loc, stmt->loc, "declaracion duplicada",
                                   "Simbolo", detalle,
                                   "solo se permite una asignacion por variable en el entorno global");
                } else {
                    /* La variable adquiere el tipo de su expresion. Si la
                       expresion tuvo error se guarda T_ERROR para que los
                       usos posteriores no reporten "no declarada". */
                    tabla_insertar(tabla, stmt->name, t, stmt->loc);
                }
                stmt->tipo = t;
                break;
            }

            case NODE_PRINT:
                stmt->tipo = check_expr(stmt->left, tabla);
                break;

            case NODE_LED: {
                Tipo t = check_expr(stmt->left, tabla);
                if (t == T_DOUBLE) {
                    semantic_error(stmt->left->loc, stmt->left->loc,
                                   "el numero de pin debe ser entero",
                                   "Tipos", "led double",
                                   "los pines digitales se identifican con un i32");
                    t = T_ERROR;
                }
                stmt->tipo = t;
                break;
            }

            default:
                break;
        }
    }

    return error_count;
}
