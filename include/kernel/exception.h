#pragma once
#include <drivers/uart.h>

void show_invalid_entry_message(int type, uint32_t esr, uint64_t elr);

void *irq_handler(void *sp);

void *default_handler(const uint32_t iar, void *sp);

void *timer_handler(uint32_t iar, void *sp);
