#include <drivers/uart.h>
#include <gic.h>

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

    volatile struct gic_distributor *gicd = (char *) GIC + 0x1000;
    volatile struct gic_controller *gicc = (char *) GIC + 0x2000;

    const uint32_t iar = gicc->IAR;

    irq_callback_type cb = get_interrupt_handler(iar);
    cb(iar);

    if (iar == 1023) return;

    gicc->EOIR = iar;
};

void default_handler(const uint32_t iar) {
    uart_puts("Interruption reçue !\r\n");
    uart_puts(" IAR: ");
    uart_put_hex(iar);
}