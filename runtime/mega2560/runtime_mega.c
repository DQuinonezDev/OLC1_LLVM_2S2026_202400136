/* Runtime bare-metal para Arduino Mega 2560 (ATmega2560 a 16 MHz).
   No usa Arduino.h: escribe directo en los registros del microcontrolador.

   - print_i32 / print_f64: envian el numero por USART0 (pines 0/1, que en
     la Mega van al chip USB) a 115200 baudios -> se ve en el monitor serial.
   - mb_led_set: enciende/apaga un pin digital 2..13 (13 = LED integrado "L").

   Se compila con -mdouble=64 (ver runtime.h). */

#include <avr/io.h>
#include <stdint.h>

#include "../runtime.h"
#include "../common/formato.h"

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define BAUD 115200UL

/* ----------------------------- USART0 ---------------------------------- */

void uart_init(void)
{
    /* Modo doble velocidad (U2X0): UBRR = F_CPU / (8 * BAUD) - 1 = 16.
       Es la misma configuracion que usa Serial.begin(115200) de Arduino. */
    UCSR0A = _BV(U2X0);
    UBRR0 = (uint16_t)(F_CPU / (8UL * BAUD) - 1UL);
    UCSR0B = _BV(TXEN0);                    /* solo transmitir */
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);     /* 8 bits, sin paridad, 1 stop */
}

static void uart_putc(char c)
{
    while (!(UCSR0A & _BV(UDRE0))) {
        /* esperar a que el buffer de transmision quede libre */
    }
    UDR0 = (uint8_t)c;
}

static void uart_line(const char *text)
{
    while (*text)
        uart_putc(*text++);
    uart_putc('\r');
    uart_putc('\n');
}

/* ------------------------ Funciones del IR ----------------------------- */

void print_i32(int32_t value)
{
    char text[16];
    formato_i32(text, sizeof text, value);
    uart_line(text);
}

void print_f64(double value)
{
    char text[32];
    formato_f64(text, sizeof text, value);
    uart_line(text);
}

/* Mapeo de pines igual al core oficial de Arduino (variante "mega").
   DDRx decide si el pin es salida; PORTx pone el nivel HIGH/LOW. */
void mb_led_set(int32_t pin, int32_t state)
{
    volatile uint8_t *ddr;
    volatile uint8_t *port;
    uint8_t mask;

    switch (pin) {
        case 2:  ddr = &DDRE; port = &PORTE; mask = _BV(PE4); break;
        case 3:  ddr = &DDRE; port = &PORTE; mask = _BV(PE5); break;
        case 4:  ddr = &DDRG; port = &PORTG; mask = _BV(PG5); break;
        case 5:  ddr = &DDRE; port = &PORTE; mask = _BV(PE3); break;
        case 6:  ddr = &DDRH; port = &PORTH; mask = _BV(PH3); break;
        case 7:  ddr = &DDRH; port = &PORTH; mask = _BV(PH4); break;
        case 8:  ddr = &DDRH; port = &PORTH; mask = _BV(PH5); break;
        case 9:  ddr = &DDRH; port = &PORTH; mask = _BV(PH6); break;
        case 10: ddr = &DDRB; port = &PORTB; mask = _BV(PB4); break;
        case 11: ddr = &DDRB; port = &PORTB; mask = _BV(PB5); break;
        case 12: ddr = &DDRB; port = &PORTB; mask = _BV(PB6); break;
        case 13: ddr = &DDRB; port = &PORTB; mask = _BV(PB7); break;
        default: return;    /* pin no soportado: se ignora */
    }

    *ddr |= mask;
    if (state)
        *port |= mask;
    else
        *port &= (uint8_t)~mask;
}
