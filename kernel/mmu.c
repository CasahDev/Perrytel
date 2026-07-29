//
// Created by sacha on 25/07/2026.
//

#include <kernel/mmu.h>

#define T0SZ (0b100000)
#define IRGN0 (0b01 << 8)
#define ORGN0 (0b01 << 10)
#define SH0 (0b11 << 12)
#define TG0 (0b00 << 14)
#define EDP1 (0b01 << 23)
#define IPS ((uint64_t) 0b010 << 32)

#define MMU_ACTIVATION 0x1
#define DATA_CACHE_ACTIVATION 0x1 << 2
#define INSTRUCT_CACHE_ACTIVATION 0x1 << 12


Table L1;
Table L2;

void mmu_init() {
    L1.pages[0] = (uint64_t) &L2 | TABLE_DESC;

    // Passe le UART en process DEVICE
    L1.pages[3] = 0xC0000000 | DESC | SHARABILITY | ACCESS_BIT | DEVICE;


    for (int i = 0; i < 512; i++) {
        L2.pages[i] = i * ((uint64_t) 1 << 21) | DESC | SHARABILITY | ACCESS_BIT;
        if (i > 503) L2.pages[i] |= DEVICE;
    }

    __asm volatile("msr TCR_EL1, %0" :: "r" (T0SZ | IRGN0 | ORGN0 | SH0 | TG0 | EDP1 | IPS));
    __asm volatile("msr MAIR_EL1, %0" :: "r" (0xFF | 0x00 << 8));
    __asm volatile("msr TTBR0_EL1, %0" :: "r" (&L1));

    __asm volatile("dsb SY");
    // Purge du TLB
    __asm volatile("tlbi VMALLE1IS");
    __asm volatile("dsb SY");

    __asm volatile("isb");

    uint64_t sctlr;
    __asm volatile("mrs %0, SCTLR_EL1" : "=r" (sctlr));

    sctlr |= MMU_ACTIVATION | DATA_CACHE_ACTIVATION | INSTRUCT_CACHE_ACTIVATION;

    __asm volatile("msr SCTLR_EL1, %0" :: "r" (sctlr));
    __asm volatile("isb");
}