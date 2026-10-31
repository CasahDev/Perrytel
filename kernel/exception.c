#include <drivers/timer.h>
#include <drivers/uart.h>

#include <kernel/exception.h>
#include <kernel/gic.h>
#include <kernel/sched.h>

#include <kernel/sio-uart.h>

void show_invalid_entry_message(int type, uint32_t esr, uint64_t elr) {
    uart_puts("Invalid exception entry: ");
    uart_put_hex(type);
    uart_puts(" ESR: ");
    uart_put_hex(esr);
    uart_puts(" ELR: ");
    uart_put_hex(elr);

    while (1);
};

void *irq_handler(void *sp) {
    // volatile struct gic_distributor *gicd = (volatile struct gic_distributor *) (GIC + 0x1000);
    volatile struct gic_controller *gicc = (volatile struct gic_controller *) (GIC + 0x2000);

    const uint32_t iar = gicc->IAR;

    irq_callback_type cb = get_interrupt_handler(iar);
    sp = cb(iar, sp);

    if (iar == 1023) return sp; 

    gicc->EOIR = iar;

    return sp;
};

void *default_handler(const uint32_t iar, void *sp) {
    uart_puts("Interruption reçue !\r\n");
    uart_puts(" IAR: ");
    uart_put_hex(iar);

    return sp;
}


void *uart_handler(uint32_t iar, void *sp) {
    (void)iar;
    uart_handle_interrupt();

    return sp;
}

void *timer_handler(uint32_t iar, void *sp) {
    (void)iar;
    timer_reset();
    set_time(get_time() + 1);
    return schedule(sp);
}

void *aux_uart_handler(uint32_t iar, void *sp) {
    (void)iar;
    sio_handler();

    return sp;
}