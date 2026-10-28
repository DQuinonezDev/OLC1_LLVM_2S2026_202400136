#include "llvm_codegen.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* Un operando LLVM ya listo para escribirse: una constante ("10",
   "2.5") o un temporal ("%t3"). */
typedef struct {
    char text[64];
} Operando;

static FILE *out;
static unsigned next_temp;      /* %t1, %t2, ... nunca se reutilizan */
static TablaSimbolos *tabla;

/* LLVM exige que una constante double tenga punto decimal ("20.0", no
   "20"). Se busca la representacion mas corta que, al leerse de nuevo, da
   exactamente el mismo valor, para que el .ll sea legible (0.1 y no
   0.10000000000000001). Valores enormes o diminutos van en notacion
   cientifica con punto: 1.5e+20. */
static void format_double(char *buffer, size_t size, double value)
{
    int use_fixed = value == 0.0 || (fabs(value) >= 1e-4 && fabs(value) < 1e15);

    for (int precision = 1; precision <= 17; precision++) {
        snprintf(buffer, size, use_fixed ? "%.*f" : "%.*e", precision, value);
        if (strtod(buffer, NULL) == value)
            break;
    }
}

static const char *llvm_type(Tipo tipo)
{
    return tipo == T_DOUBLE ? "double" : "i32";
}

/* Tabla 4.5 del enunciado: el tipo elige la instruccion. */
static const char *llvm_instruction(char op, Tipo tipo)
{
    int is_double = tipo == T_DOUBLE;
    switch (op) {
        case '+': return is_double ? "fadd" : "add";
        case '-': return is_double ? "fsub" : "sub";
        case '*': return is_double ? "fmul" : "mul";
        case '/': return is_double ? "fdiv" : "sdiv";
        default:  return NULL;
    }
}

/* Recorrido post-orden (Figura 8): primero se resuelven los hijos y
   luego el padre emite su instruccion consumiendo los operandos que
   dejaron. Las hojas no emiten nada: se usan tal cual. */
static Operando gen_expr(const ASTNode *node)
{
    Operando result;

    switch (node->type) {
        case NODE_ENTERO:
            snprintf(result.text, sizeof result.text, "%d", node->ival);
            break;

        case NODE_DECIMAL:
            format_double(result.text, sizeof result.text, node->dval);
            break;

        case NODE_ID: {
            /* La tabla guarda la constante o temporal que produjo el let. */
            Simbolo *s = tabla_buscar(tabla, node->name);
            snprintf(result.text, sizeof result.text, "%s", s ? s->valor : "undef");
            break;
        }

        case NODE_BINOP: {
            Operando left = gen_expr(node->left);
            Operando right = gen_expr(node->right);
            snprintf(result.text, sizeof result.text, "%%t%u", ++next_temp);
            fprintf(out, "  %s = %s %s %s, %s\n", result.text,
                    llvm_instruction(node->op, node->tipo), llvm_type(node->tipo),
                    left.text, right.text);
            break;
        }

        default:
            strcpy(result.text, "undef");
            break;
    }
    return result;
}

int llvm_generate(ASTNode *root, TablaSimbolos *symbols, FILE *output,
                  const char *entry, const char *source_name)
{
    out = output;
    tabla = symbols;
    next_temp = 0;

    /* Comprobacion defensiva: el generador solo acepta un AST tipado. */
    for (const ASTNode *stmt = root; stmt; stmt = stmt->next)
        if (stmt->tipo != T_INT && stmt->tipo != T_DOUBLE) {
            fputs("Error interno: se intento generar LLVM IR de un AST con errores.\n", stderr);
            return 1;
        }

    /* No se fija target triple: el mismo .ll sirve para AVR, ARM, RISC-V o
       la PC. El destino se elige despues con "llc -mtriple=...". */
    fprintf(out, "; LLVM IR generado por el compilador OLC1 a partir de %s\n", source_name);
    fprintf(out, "source_filename = \"%s\"\n\n", source_name);
    fputs("; Funciones del runtime de la placa (salida por el monitor serial)\n", out);
    fputs("declare void @print_i32(i32)\n", out);
    fputs("declare void @print_f64(double)\n", out);
    fputs("declare void @mb_led_set(i32, i32)\n\n", out);
    fprintf(out, "define void @%s() {\nentry:\n", entry);

    for (const ASTNode *stmt = root; stmt; stmt = stmt->next) {
        switch (stmt->type) {
            case NODE_LET: {
                fprintf(out, "  ; linea %d: let %s : %s\n", stmt->loc.line, stmt->name, tipo_nombre(stmt->tipo));
                Operando value = gen_expr(stmt->left);
                Simbolo *s = tabla_buscar(tabla, stmt->name);
                if (s)
                    snprintf(s->valor, sizeof s->valor, "%s", value.text);
                break;
            }

            case NODE_PRINT: {
                fprintf(out, "  ; linea %d: print : %s\n", stmt->loc.line, tipo_nombre(stmt->tipo));
                Operando value = gen_expr(stmt->left);
                if (stmt->tipo == T_DOUBLE)
                    fprintf(out, "  call void @print_f64(double %s)\n", value.text);
                else
                    fprintf(out, "  call void @print_i32(i32 %s)\n", value.text);
                break;
            }

            case NODE_LED: {
                fprintf(out, "  ; linea %d: led %s\n", stmt->loc.line, stmt->ival ? "on" : "off");
                Operando pin = gen_expr(stmt->left);
                fprintf(out, "  call void @mb_led_set(i32 %s, i32 %d)\n", pin.text, stmt->ival);
                break;
            }

            default:
                break;
        }
    }

    fputs("  ret void\n}\n", out);
    return 0;
}
