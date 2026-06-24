#include <drivers/uart.h>
#include <gic.h>
#include <exception.h>

irq_callback_type interruption_handlers[1024];

void init_interruption_handlers() {
    for (int i = 0; i < 1024; i++) {
        interruption_handlers[i] = default_handler;
    }
}

void gic_init() {
    volatile struct gic_distributor *gicd = (char *) GIC + 0x1000;
    volatile struct gic_controller *gicc = (char *) GIC + 0x2000;

    gicd->CTLR = 0x01;
    gicc->CTLR = 0x01;

    gicc->PMR = 0xFFFF;

    init_interruption_handlers();
}

void register_interrupt(uint32_t irq_id, irq_callback_type handler) {
    interruption_handlers[irq_id] = handler;
}

irq_callback_type get_interrupt_handler(uint32_t irq_id) {
    return interruption_handlers[irq_id];
}