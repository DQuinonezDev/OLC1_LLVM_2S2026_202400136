/* Runtime para la PC: permite ejecutar el mismo .ll en Windows/Linux y ver
   la salida en la consola, sin necesitar la placa. Usa el mismo formato de
   numeros que la Mega, asi la salida debe ser identica a la del monitor serial. */

#include <stdio.h>

#include "../runtime.h"
#include "../common/formato.h"

void print_i32(int32_t value)
{
    char text[16];
    formato_i32(text, sizeof text, value);
    puts(text);
}

void print_f64(double value)
{
    char text[32];
    formato_f64(text, sizeof text, value);
    puts(text);
}

void mb_led_set(int32_t pin, int32_t state)
{
    printf("[LED] pin %ld -> %s\n", (long)pin, state ? "ON" : "OFF");
}

int main(void)
{
    minibot_program();
    return 0;
}
