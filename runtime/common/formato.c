#include "formato.h"

typedef struct {
    char *buffer;
    size_t size;
    size_t length;
} Texto;

static void put_char(Texto *t, char c)
{
    if (t->length + 1 < t->size) {
        t->buffer[t->length++] = c;
        t->buffer[t->length] = '\0';
    }
}

static void put_text(Texto *t, const char *s)
{
    while (*s)
        put_char(t, *s++);
}

static void put_unsigned(Texto *t, uint64_t value)
{
    char reversed[20];
    int count = 0;
    do {
        reversed[count++] = (char)('0' + value % 10u);
        value /= 10u;
    } while (value != 0u);
    while (count > 0)
        put_char(t, reversed[--count]);
}

/* Escribe ".dddddd" quitando ceros finales, pero deja al menos un
   digito para que un decimal siempre se distinga de un entero. */
static void put_fraction(Texto *t, uint32_t fraction)
{
    char digits[6];
    for (int i = 5; i >= 0; i--) {
        digits[i] = (char)('0' + fraction % 10u);
        fraction /= 10u;
    }
    int count = 6;
    while (count > 1 && digits[count - 1] == '0')
        count--;
    put_char(t, '.');
    for (int i = 0; i < count; i++)
        put_char(t, digits[i]);
}

void formato_i32(char *buffer, size_t size, int32_t value)
{
    Texto t = { buffer, size, 0 };
    if (size == 0)
        return;
    buffer[0] = '\0';

    if (value < 0) {
        put_char(&t, '-');
        put_unsigned(&t, 0u - (uint32_t)value);
    } else {
        put_unsigned(&t, (uint32_t)value);
    }
}

void formato_f64(char *buffer, size_t size, double value)
{
    Texto t = { buffer, size, 0 };
    if (size == 0)
        return;
    buffer[0] = '\0';

    if (value != value) {
        put_text(&t, "nan");
        return;
    }
    if (value < 0) {
        put_char(&t, '-');
        value = -value;
    }
    if (value > 1.7976931348623157e308) {
        put_text(&t, "inf");
        return;
    }

    /* Numeros muy grandes o muy pequenos: notacion cientifica d.dddddde+N */
    if (value >= 1e15 || (value != 0 && value < 1e-6)) {
        int exponent = 0;
        while (value >= 10.0) { value /= 10.0; exponent++; }
        while (value < 1.0)   { value *= 10.0; exponent--; }
        uint32_t scaled = (uint32_t)(value * 1e6 + 0.5);
        if (scaled >= 10000000u) { scaled /= 10u; exponent++; }
        put_unsigned(&t, scaled / 1000000u);
        put_fraction(&t, scaled % 1000000u);
        put_char(&t, 'e');
        if (exponent < 0) { put_char(&t, '-'); exponent = -exponent; }
        else              { put_char(&t, '+'); }
        put_unsigned(&t, (uint32_t)exponent);
        return;
    }

    uint64_t integer = (uint64_t)value;
    uint32_t fraction = (uint32_t)((value - (double)integer) * 1e6 + 0.5);
    if (fraction >= 1000000u) {
        integer++;
        fraction -= 1000000u;
    }
    put_unsigned(&t, integer);
    put_fraction(&t, fraction);
}
