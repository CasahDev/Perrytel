#ifndef UART_H
#define UART_H

#include <stdint.h>

enum
{
    // The offsets for reach register.
    GPIO_BASE = 0x200000,
    GPFSEL1 = (GPIO_BASE + 0x04),
    GPPUD = (GPIO_BASE + 0x94),
    GPPUDCLK0 = (GPIO_BASE + 0x98),
    UART0_BASE = (GPIO_BASE + 0x1000),
    UART0_DR     = (UART0_BASE + 0x00),
    UART0_RSRECR = (UART0_BASE + 0x04),
    UART0_FR     = (UART0_BASE + 0x18),
    UART0_ILPR   = (UART0_BASE + 0x20),
    UART0_IBRD   = (UART0_BASE + 0x24),
    UART0_FBRD   = (UART0_BASE + 0x28),
    UART0_LCRH   = (UART0_BASE + 0x2C),
    UART0_CR     = (UART0_BASE + 0x30),
    UART0_IMSC   = (UART0_BASE + 0x38),
    UART0_MIS    = (UART0_BASE + 0x40),
    UART0_ICR    = (UART0_BASE + 0x44),
    MBOX_BASE    = 0xB880,
    MBOX_STATUS  = (MBOX_BASE + 0x18),
    MBOX_WRITE   = (MBOX_BASE + 0x20),
    MBOX_READ    = (MBOX_BASE + 0x00)
};

void uart_init(int raspi);
void uart_putc(unsigned char c);
unsigned char uart_getc();
void uart_puts(const char* str);
void uart_put_hex(uint64_t hex);
void uart_handle_interrupt();

#endif
