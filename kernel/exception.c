#include <drivers/uart.h>

void show_invalid_entry_message(int type, uint32_t esr, uint64_t elr) {
    uart_puts("Invalid exception entry: ");
    uart_put_hex(type);
    uart_puts(" ESR: ");
    uart_put_hex(esr);
    uart_puts(" ELR: ");
    uart_put_hex(elr);

    while (1);
};

void irq_handler(void) {
    uart_puts("Interruption reçue !\r\n");
}