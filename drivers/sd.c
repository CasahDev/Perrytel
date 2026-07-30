//
// Created by sacha on 30/07/2026.
//
#include <stdint.h>
#include <drivers/sd.h>

#include <kernel/mmio.h>

#define GLOBAL_RESET 1 << 24
#define CONTROL1_POS 0x2C
#define INTERNAL_CLOCK_ENABLE 0x1
#define INTERNAL_CLOCK_STABLE 0x1 << 1
#define SD_CLOCK_ENABLE 0x1 << 2
#define CLOCK_DEVISER 0x7C << 8


void sd_init() {
    uint32_t control1 = mmio_read(EMMC, CONTROL1_POS);
    control1 |= GLOBAL_RESET;
    mmio_write(EMMC, CONTROL1_POS, control1);
    while ((mmio_read(EMMC, CONTROL1_POS) >> 24 & 0x1) == 1) {}

    control1 = mmio_read(EMMC, CONTROL1_POS);
    control1 |= INTERNAL_CLOCK_ENABLE;
    control1 &= ~(0xFF << 8);
    control1 |= CLOCK_DEVISER;
    mmio_write(EMMC, CONTROL1_POS, control1);
    while ((mmio_read(EMMC, CONTROL1_POS) >> 1 & 0x1) == 0) {}

    control1 = mmio_read(EMMC, CONTROL1_POS);
    control1 |= SD_CLOCK_ENABLE;
    mmio_write(EMMC, CONTROL1_POS, control1);
}