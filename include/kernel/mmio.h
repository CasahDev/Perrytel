#pragma once
#include <stdint.h>

extern uint32_t MMIO_BASE;

static void mmio_write(uint32_t start, uint32_t reg, uint32_t data) {
    *(volatile uint32_t*)(uintptr_t)(start + reg) = data;
}

static uint32_t mmio_read(uint32_t start, uint32_t reg) {
    return *(volatile uint32_t*)(uintptr_t)(start + reg);
}