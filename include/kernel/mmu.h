//
// Created by sacha on 25/07/2026.
//

#ifndef PERRYTEL_MMU_H
#define PERRYTEL_MMU_H
#include <stdint.h>

#define TABLE_DESC 0x3
#define DESC 0x1
#define ACCESS_BIT 0x400
#define SHARABILITY 0x300
#define DEVICE 0x4

typedef struct Table {
    uint64_t pages[512];
} Table __attribute__ ((aligned (4096)));

void mmu_init();

#endif //PERRYTEL_MMU_H