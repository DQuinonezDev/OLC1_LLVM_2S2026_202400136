#include "../runtime.h"

void uart_init(void);

int main(void)
{
    uart_init();
    minibot_program();      /* el codigo que genero nuestro compilador */

    while (1) {
        /* un microcontrolador nunca "termina": se queda aqui */
    }
}
