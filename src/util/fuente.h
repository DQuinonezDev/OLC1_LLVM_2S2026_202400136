#ifndef FUENTE_H
#define FUENTE_H

#include "ast.h"

/* Guarda en memoria las lineas del archivo fuente para poder mostrarlas
   en los mensajes de error, subrayando con ^^^ la parte problematica. */
int fuente_cargar(const char *path);
void fuente_subrayar(Ubicacion loc);
void fuente_liberar(void);
const char *fuente_nombre(void);

#endif
