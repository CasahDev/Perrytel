#include <drivers/uart.h>
#include <gic.h>

void show_invalid_entry_message(int type, uint32_t esr, uint64_t elr);

void irq_handler(void);

void default_handler(void);