#include <stdint.h>
#include <drivers/uart.h>

#ifdef AARCH64
void kernel_main(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
{
    (void)dtb_ptr32; (void)x1; (void)x2; (void)x3;
#else
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    (void)r0; (void)r1; (void)atags;
#endif

    uart_init(4);
    uart_puts("Perrytel OS (AArch64) - Kernel Loaded\r\n");

    __asm__ volatile("brk #0");

    while (1) {
        unsigned char c = uart_getc();
        if (c == '\r') {
            uart_putc('\r');
            uart_putc('\n');
        } else {
            uart_putc(c);
        }
    }
}
