#pragma once
#include <drivers/uart.h>

enum {
    BASE_INTERRUPT_VALUE = 96,
    UART_INT = (BASE_INTERRUPT_VALUE + 57),
    UART_AUX_INT = (BASE_INTERRUPT_VALUE + 29),
    TIMER_INT = 27,
};

void show_invalid_entry_message(int type, uint32_t esr, uint64_t elr);

void *irq_handler(void *sp);

void *default_handler(const uint32_t iar, void *sp);

void *timer_handler(uint32_t iar, void *sp);

void *aux_uart_handler(uint32_t iar, void *sp);
