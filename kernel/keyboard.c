#include <drivers/uart.h>

volatile char state;

void keyboard_update() {
    int c = uart_get_char_async();
    if (c != -1) {
        if (!state) {
            if (c == 0x13) {
                state = 1;
            } else if (c == '\r') {
                    uart_putc('\r');
                    uart_putc('\n');
            } else {
                uart_putc(c);
            }
            
        } else {
            if (c == 0x41) {
                uart_putc('\r');
                uart_putc('\n');
            } else if (c == 0x42) {
                
            }
            state = 0;
        }
    }
}