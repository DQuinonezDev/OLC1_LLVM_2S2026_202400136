#ifndef LLVM_CODEGEN_H
#define LLVM_CODEGEN_H

#include "ast.h"
#include "symtab.h"
#include <stdio.h>

/* Traduce a LLVM IR textual un AST que YA paso el analisis semantico.
   - entry: nombre de la funcion que contendra el programa; el runtime de
     la placa la llama desde su main() (por defecto "minibot_program").
   Llena la columna "valor" de la tabla de simbolos. Devuelve 0 si todo
   salio bien. */
int llvm_generate(ASTNode *root, TablaSimbolos *tabla, FILE *out,
                  const char *entry, const char *source_name);

#endif
