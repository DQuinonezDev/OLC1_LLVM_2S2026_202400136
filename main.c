#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yyparse(void);
int yylex_destroy(void);
extern FILE *yyin;
extern ASTNode *root;
extern int lexical_error;
extern int syntax_error;

int main(int argc, char **argv)
{
    int only_ast = argc == 3 && strcmp(argv[1], "--ast") == 0;
    if (!(argc == 2 || only_ast)) {
        fprintf(stderr, "Uso: %s [--ast] entrada.olc\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *path = argv[only_ast ? 2 : 1];
    yyin = fopen(path, "r");
    if (!yyin) {
        perror(path);
        return EXIT_FAILURE;
    }

    int result = yyparse();
    fclose(yyin);
    yylex_destroy();

    if (result != 0 || lexical_error || syntax_error) {
        fputs("Compilacion detenida: hay errores lexicos o sintacticos.\n", stderr);
        ast_free(root);
        return EXIT_FAILURE;
    }

    puts("=== AST ===");
    ast_print(root, 0);

    ast_free(root);
    return EXIT_SUCCESS;
}
