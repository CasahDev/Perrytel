#include <stdint.h>

#include <drivers/sd.h>
#include <drivers/timer.h>
#include <drivers/uart.h>

#include <kernel/exception.h>
#include <kernel/gic.h>
#include <kernel/mmu.h>
#include <kernel/shell.h>
#include <kernel/syscall.h>

#include "lwip/netif.h"
#include "lwip/init.h"
#include "lwip/ip.h"
#include "netif/slipif.h"

struct netif net;

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
    register_interrupt(UART_INT, uart_handler);
    register_interrupt(UART_AUX_INT, aux_uart_handler);
    timer_init();
    register_interrupt(TIMER_INT, timer_handler);

    init_syscall_handler();
    init_fd();
    register_syscall_handler(SYS_READ, sys_read);
    register_syscall_handler(SYS_WRITE, uart_write);
    register_syscall_handler(SYS_EXIT, sys_exit_handler);
    register_syscall_handler(SYS_OPEN, sys_open);
    register_syscall_handler(SYS_CLOSE, sys_close);

    sd_init();

    lwip_init();

#ifdef QEMU
    ip4_addr_t ip;
    IP4_ADDR(&ip, 192, 168, 5, 253);

    ip4_addr_t mask;
    IP4_ADDR(&mask, 255, 255, 255, 0);

    ip4_addr_t gateway;
    IP4_ADDR(&gateway, 192, 168, 5, 254);

    int state = 1;

    netif_add(&net, &ip, &mask, &gateway, &state, slipif_init, ip_input);

    netif_set_default(&net);
    netif_set_up(&net);

#endif

    init_shell(*active_fs);

    uart_puts("Perrytel OS (AArch64) - Kernel Loaded\r\n");
    run_shell();
}
