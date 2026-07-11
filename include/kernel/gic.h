#include <stdint.h>
#ifdef QEMU
#define GIC 0xFF840000
#else
#define GIC 0x400040000
#endif

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

typedef void (*irq_callback_type) (uint32_t);


void gic_init();
void register_interrupt(uint32_t timer_id, irq_callback_type handler);
irq_callback_type get_interrupt_handler(uint32_t timer_id);
void keyboard_callback(uint32_t iar);