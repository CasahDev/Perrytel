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


Table L1;
Table L2;

void mmu_init() {
    L1.pages[0] = (uint64_t) &L2 | TABLE_DESC;

    for (int i = 0; i < 512; i++) {
        if (i == 0) {
            L2.pages[0] = 0;
        } else {
            L2.pages[i] = i * ((uint64_t) 1 << 21) | DESC | SHARABILITY | ACCESS_BIT;
            if (i > 503) L2.pages[i] |= DEVICE;
        }
    }

    __asm volatile("msr TCR_EL1, %0" :: "r" (T0SZ | IRGN0 | ORGN0 | SH0 | TG0 | EDP1 | IPS));
}