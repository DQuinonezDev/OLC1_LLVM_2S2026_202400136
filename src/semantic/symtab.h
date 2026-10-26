#ifndef SYMTAB_H
#define SYMTAB_H

#include "ast.h"

/* Una fila de la tabla de simbolos (Figura 3 del enunciado):
     nombre | tipo   | valor
     precio | double | 25.5
     total  | double | %t1                                         */
typedef struct {
    char *nombre;
    Tipo tipo;          /* lo decide el analisis semantico */
    char valor[64];     /* lo llena el generador: constante o temporal LLVM */
    Ubicacion decl;     /* donde se declaro (para errores de duplicado) */
} Simbolo;

/* Unico entorno global: el lenguaje no tiene bloques ni funciones. */
typedef struct {
    Simbolo *items;
    int count;
    int capacity;
} TablaSimbolos;

void tabla_init(TablaSimbolos *tabla);
Simbolo *tabla_buscar(TablaSimbolos *tabla, const char *nombre);
Simbolo *tabla_insertar(TablaSimbolos *tabla, const char *nombre, Tipo tipo, Ubicacion decl);
void tabla_imprimir(const TablaSimbolos *tabla);
void tabla_liberar(TablaSimbolos *tabla);

#endif
