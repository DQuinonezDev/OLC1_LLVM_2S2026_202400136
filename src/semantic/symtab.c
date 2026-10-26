#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tabla_init(TablaSimbolos *tabla)
{
    tabla->items = NULL;
    tabla->count = 0;
    tabla->capacity = 0;
}

Simbolo *tabla_buscar(TablaSimbolos *tabla, const char *nombre)
{
    for (int i = 0; i < tabla->count; i++)
        if (strcmp(tabla->items[i].nombre, nombre) == 0)
            return &tabla->items[i];
    return NULL;
}

Simbolo *tabla_insertar(TablaSimbolos *tabla, const char *nombre, Tipo tipo, Ubicacion decl)
{
    if (tabla->count == tabla->capacity) {
        tabla->capacity = tabla->capacity ? tabla->capacity * 2 : 16;
        tabla->items = realloc(tabla->items, (size_t)tabla->capacity * sizeof *tabla->items);
        if (!tabla->items) {
            fputs("No hay memoria disponible.\n", stderr);
            exit(EXIT_FAILURE);
        }
    }

    Simbolo *s = &tabla->items[tabla->count++];
    s->nombre = malloc(strlen(nombre) + 1);
    if (!s->nombre) {
        fputs("No hay memoria disponible.\n", stderr);
        exit(EXIT_FAILURE);
    }
    strcpy(s->nombre, nombre);
    s->tipo = tipo;
    s->valor[0] = '\0';
    s->decl = decl;
    return s;
}

void tabla_imprimir(const TablaSimbolos *tabla)
{
    printf("%-16s %-8s %-24s %s\n", "NOMBRE", "TIPO", "VALOR", "LINEA");
    for (int i = 0; i < tabla->count; i++) {
        const Simbolo *s = &tabla->items[i];
        printf("%-16s %-8s %-24s %d\n", s->nombre, tipo_nombre(s->tipo),
               s->valor[0] ? s->valor : "-", s->decl.line);
    }
}

void tabla_liberar(TablaSimbolos *tabla)
{
    for (int i = 0; i < tabla->count; i++)
        free(tabla->items[i].nombre);
    free(tabla->items);
    tabla_init(tabla);
}
