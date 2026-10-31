#include <kernel/exception.h>
#include <kernel/gic.h>

#define INTERRUPTION_HANDLERS 1024

irq_callback_type interruption_handlers[INTERRUPTION_HANDLERS];

volatile struct gic_distributor *gicd;
volatile struct gic_controller *gicc;

void init_interruption_handlers() {
    for (int i = 0; i < INTERRUPTION_HANDLERS; i++) {
        interruption_handlers[i] = default_handler;
    }
}

void gic_enable_interrupt(const uint32_t irq_id, const uint8_t cpu_target) {
    gicd->ISENABLER[GIC_ISENABLER_INDEX(irq_id)] |= GIC_ISENABLER_BIT(irq_id);

    if (irq_id >= 32) {
        gicd->ITARGETSR[irq_id / 4] |= (cpu_target << ((irq_id % 4) * 8));
    }
}

void gic_init() {
    gicd = (struct gic_distributor *) ((char *) GIC + DISTRIBUTOR_OFFSET);
    gicc = (struct gic_controller *) ((char *) GIC + CONTROLLER_OFFSET);

    gicd->CTLR = GIC_ENABLE_BIT;
    gicc->CTLR = GIC_ENABLE_BIT;

    gicc->PMR = 0xFFFF;

    // Active les interuptions clavier
    gic_enable_interrupt(UART_INT, 0x01);

    // Active les interruptions Timer 
    gic_enable_interrupt(TIMER_INT, 0x01);

    // Enable mini uart interrupts
    gic_enable_interrupt(UART_AUX_INT, 0x01);

    init_interruption_handlers();
}

void register_interrupt(uint32_t irq_id, irq_callback_type handler) {
    interruption_handlers[irq_id] = handler;
}

irq_callback_type get_interrupt_handler(uint32_t irq_id) {
    return interruption_handlers[irq_id];
}
