#include <stdint.h>
#include <drivers/uart.h>
#include <kernel/gic.h>
#include <drivers/timer.h>
#include <kernel/exception.h>
#include <kernel/user.h>

#include <kernel/mmu.h>

#include "sd.h"

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

    gic_init();

    mmu_init();

    __asm__ volatile("msr daifclr, #2");
    register_interrupt(153, uart_handler);
    timer_init();
    register_interrupt(27, timer_handler);

    uart_puts("Perrytel OS (AArch64) - Kernel Loaded\r\n");

    uart_puts("Perry -> ");

    sd_init();

    user_mode();
}
