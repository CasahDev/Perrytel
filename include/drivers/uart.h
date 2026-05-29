#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init(int raspi);
void uart_putc(unsigned char c);
unsigned char uart_getc();
void uart_puts(const char* str);
void uart_put_hex(uint64_t hex);

#endif
