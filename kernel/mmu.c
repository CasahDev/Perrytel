//
// Created by sacha on 25/07/2026.
//

#include <kernel/mmu.h>

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
}