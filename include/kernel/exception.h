#include <drivers/uart.h>

void show_invalid_entry_message(int type, uint32_t esr, uint64_t elr);

void irq_handler(void);

void default_handler(const uint32_t iar);