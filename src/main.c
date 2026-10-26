#include "ast.h"
#include "fuente.h"
#include "llvm_codegen.h"
#include "semantic.h"
#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Funciones y datos generados por Flex y Bison. */
int yyparse(void);
int yylex_destroy(void);
extern FILE *yyin;
extern ASTNode *root;
extern int lexical_error;
extern int syntax_error;

typedef struct {
    const char *input;
    const char *output;     /* NULL: mismo nombre que la entrada con .ll */
    const char *entry;
    int show_ast;
    int show_table;
} Opciones;

static void print_usage(const char *program)
{
    fprintf(stderr,
            "Uso: %s [opciones] entrada.olc\n"
            "  -o salida.ll     archivo LLVM IR a generar (por defecto entrada.ll)\n"
            "  --ast            muestra el AST tipado\n"
            "  --tabla          muestra la tabla de simbolos\n"
            "  --entry nombre   nombre de la funcion generada (por defecto minibot_program)\n",
            program);
}

static int parse_arguments(int argc, char **argv, Opciones *op)
{
    memset(op, 0, sizeof *op);
    op->entry = "minibot_program";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--ast") == 0)
            op->show_ast = 1;
        else if (strcmp(argv[i], "--tabla") == 0)
            op->show_table = 1;
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
            op->output = argv[++i];
        else if (strcmp(argv[i], "--entry") == 0 && i + 1 < argc)
            op->entry = argv[++i];
        else if (argv[i][0] != '-' && !op->input)
            op->input = argv[i];
        else
            return 0;
    }
    return op->input != NULL;
}

/* "examples/area.olc" -> "examples/area.ll" */
static char *default_output(const char *input)
{
    const char *slash = strrchr(input, '/');
    const char *backslash = strrchr(input, '\\');
    const char *base = slash > backslash ? slash : backslash;
    base = base ? base + 1 : input;
    const char *dot = strrchr(base, '.');
    size_t prefix = dot ? (size_t)(dot - input) : strlen(input);

    char *path = malloc(prefix + sizeof ".ll");
    if (!path) {
        fputs("No hay memoria disponible.\n", stderr);
        exit(EXIT_FAILURE);
    }
    memcpy(path, input, prefix);
    memcpy(path + prefix, ".ll", sizeof ".ll");
    return path;
}

int main(int argc, char **argv)
{
    Opciones op;
    if (!parse_arguments(argc, argv, &op)) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    yyin = fopen(op.input, "r");
    if (!yyin || !fuente_cargar(op.input)) {
        perror(op.input);
        return EXIT_FAILURE;
    }

    /* FASE 1 y 2 - LEXICO Y SINTACTICO.
       yyparse() pide tokens a yylex() (Flex) y aplica la gramatica (Bison).
       Las acciones de cada produccion construyen el AST en "root". */
    int parse_result = yyparse();
    fclose(yyin);
    yylex_destroy();

    if (parse_result != 0 || lexical_error || syntax_error) {
        fputs("Compilacion detenida: hay errores lexicos o sintacticos.\n", stderr);
        ast_free(root);
        fuente_liberar();
        return EXIT_FAILURE;
    }

    /* FASE 3 - ANALISIS SEMANTICO: tipos y tabla de simbolos. */
    TablaSimbolos tabla;
    tabla_init(&tabla);
    int errors = semantic_analyze(root, &tabla);

    if (op.show_ast) {
        puts("=== AST ===");
        ast_print(root);
        putchar('\n');
    }

    if (errors > 0) {
        fprintf(stderr, "Compilacion detenida: %d error(es) semantico(s). No se genero LLVM IR.\n", errors);
        if (op.show_table) {
            puts("=== TABLA DE SIMBOLOS ===");
            tabla_imprimir(&tabla);
        }
        tabla_liberar(&tabla);
        ast_free(root);
        fuente_liberar();
        return EXIT_FAILURE;
    }

    /* FASE 4 - GENERACION DE LLVM IR: solo con un AST sin errores. */
    char *allocated = op.output ? NULL : default_output(op.input);
    const char *output_path = op.output ? op.output : allocated;
    FILE *out = fopen(output_path, "w");
    int ok = out != NULL;
    if (!out)
        perror(output_path);
    else {
        const char *base = strrchr(op.input, '/');
        ok = llvm_generate(root, &tabla, out, op.entry, base ? base + 1 : op.input) == 0;
        fclose(out);
    }

    if (ok) {
        if (op.show_table) {
            puts("=== TABLA DE SIMBOLOS ===");
            tabla_imprimir(&tabla);
            putchar('\n');
        }
        printf("LLVM IR generado: %s\n", output_path);
    }

    free(allocated);
    tabla_liberar(&tabla);
    ast_free(root);
    fuente_liberar();
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
