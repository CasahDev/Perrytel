#pragma once
#include <stdint.h>
#ifdef QEMU
#define GIC 0xFF840000
#else
#define GIC 0x400040000
#endif

#define DISTRIBUTOR_OFFSET 0x1000
#define CONTROLLER_OFFSET 0x2000


#define GIC_ISENABLER_INDEX(irq) (irq / 32)
#define GIC_ISENABLER_BIT(irq) (1 << (irq % 32))
#define GIC_ITARGETSR_INDEX(irq) (irq/4)
#define GIC_ITARGETSR_SHIFT(irq) ((irq % 4) * 8)

enum {
    // Activation du controller GIC
    GIC_ENABLE_BIT = 0x01,

    // Demasquage de tous les niveaux de priorité
    GIC_PMR_UNMASK_ALL = 0xFFFF,
};

struct gic_distributor
{
    uint32_t CTLR;     // Control register
    const    uint32_t TYPER;    // Interrupt controller Type Register
    const    uint32_t IIDR;     // Implementer Identification Register
    uint32_t _reserved0[29];    // Padding

    uint32_t IGROUPR[32];      // Interrupt Group Registers

    uint32_t ISENABLER[32];    // Interrupt Group Registers
    uint32_t ICENABLER[32];    // Interrupt Group Registers
    uint32_t ISPENDR[32];
    uint32_t ICPENDR[32];
    uint32_t ISACTIVER[32];
    uint32_t ICACTIVER[32];
    uint32_t IPRIORITYR[256];
    uint32_t ITARGETSR[256];
    uint32_t ICFGR[64];
    uint32_t _reserved1[128];
    uint32_t SGIR;
};

struct gic_controller 
{
    uint32_t CTLR;
    uint32_t PMR;
    uint32_t BPR;
    const uint32_t IAR;
    uint32_t EOIR;
};

typedef void *(*irq_callback_type) (uint32_t, void*);


void gic_init();
void register_interrupt(uint32_t handler_id, irq_callback_type handler);
irq_callback_type get_interrupt_handler(uint32_t handler_id);
void *uart_handler(uint32_t iar, void *sp);