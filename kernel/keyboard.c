#include <stdbool.h>

#include <drivers/uart.h>

volatile bool is_function_state;

void keyboard_update() {
    int c = uart_get_char_async();
    if (c != -1) {
        if (!is_function_state) {
            if (c == 0x13) {
                is_function_state = true;
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
            is_function_state = false;
        }
    }
}