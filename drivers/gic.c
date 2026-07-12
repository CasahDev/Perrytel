#include <drivers/uart.h>
#include <kernel/gic.h>
#include <kernel/exception.h>

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
    
    // Active les interuptions clavier
    gicd->ISENABLER[4] |= (1 << 25);
    gicd->ITARGETSR[38] |= (1 << 8);

    // Active les interruptions Timer 
    gicd->ISENABLER[0] |= (1 << 27);


    init_interruption_handlers();
}

void register_interrupt(uint32_t irq_id, irq_callback_type handler) {
    interruption_handlers[irq_id] = handler;
}

irq_callback_type get_interrupt_handler(uint32_t irq_id) {
    return interruption_handlers[irq_id];
}