#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symtab.h"

/* Recorre el AST completo, anota el tipo de cada expresion y llena la
   tabla de simbolos. Reporta todos los errores encontrados (no se detiene
   en el primero) y devuelve cuantos hubo. Si devuelve 0, el AST esta
   listo para el generador de LLVM IR. */
int semantic_analyze(ASTNode *root, TablaSimbolos *tabla);

#endif
