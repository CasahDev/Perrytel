#include <drivers/uart.h>
#include <kernel/gic.h>
#include <kernel/exception.h>
#include <drivers/timer.h>

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
    volatile struct gic_distributor *gicd = (volatile struct gic_distributor *) (GIC + 0x1000);
    volatile struct gic_controller *gicc = (volatile struct gic_controller *) (GIC + 0x2000);

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


void keyboard_callback(uint32_t iar) {
    // 1. On récupère la touche (et ça acquitte l'interruption au niveau de l'UART)
    unsigned char c = uart_getc();
    
    if (c == '\r') {
        uart_putc('\r');
        uart_putc('\n');
    } else {
        uart_putc(c);
    }
}

void timer_handler(uint32_t iar) {
    timer_reset();
}