#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdint.h>

/* Contrato (ABI) entre el LLVM IR generado y el runtime de cada placa.
   El .ll solo DECLARA estas funciones; cada plataforma las IMPLEMENTA.

   IMPORTANTE en AVR: LLVM trata "double" como 64 bits, pero avr-gcc usa
   32 bits por defecto. Por eso el runtime de la Mega se compila con
   -mdouble=64, para que ambos lados hablen del mismo tipo. */

void print_i32(int32_t value);
void print_f64(double value);
void mb_led_set(int32_t pin, int32_t state);

/* Funcion generada por el compilador (nombre configurable con --entry). */
void minibot_program(void);

#endif
