#ifndef FORMATO_H
#define FORMATO_H

#include <stddef.h>
#include <stdint.h>

/* Convierte numeros a texto sin usar printf (en AVR, printf con %f
   necesita una biblioteca extra y ocupa mucha memoria).
   Decimales: hasta 6 cifras, sin ceros sobrantes: 210.0, 3.75, 0.333333 */
void formato_i32(char *buffer, size_t size, int32_t value);
void formato_f64(char *buffer, size_t size, double value);

#endif
