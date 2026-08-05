//
// Created by sacha on 04/08/2026.
//


#include <stdint.h>

#include <drivers/uart.h>

#include <kernel/syscall.h>

syscall_callback_type syscall_handlers[1024];

static uint32_t default_handler(const uint32_t syscall_id, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    uart_puts("System call reçue !\r\n");
    uart_puts(" ID: ");
    uart_put_hex(syscall_id);
    uart_puts(" Arg0: ");
    uart_put_hex(arg0);
    uart_puts("\r\n");
    uart_puts(" Arg1: ");
    uart_put_hex(arg1);
    uart_puts("\r\n");
    uart_puts(" Arg2: ");
    uart_put_hex(arg2);

    return 0;
}


void init_syscall_handler() {
    for (int i = 0; i < 1024; i++) {
        syscall_handlers[i] = default_handler;
    }
}


void *el0_sync_handler(void *sp) {
    uint32_t esr_el1;
    __asm volatile("mrs %0, ESR_EL1" : "=r" (esr_el1));

    if (((esr_el1 >> 26) & 0x3F) == 0x15) {
        uint64_t *regs = sp;
        uint64_t syscall_id = regs[8];
        uint64_t x0 = regs[0];
        uint64_t x1 = regs[1];
        uint64_t x2 = regs[2];
        regs[0] = get_syscall_handler(syscall_id)(syscall_id, x0, x1, x2);
    }

    return sp;
}

syscall_callback_type get_syscall_handler(uint32_t syscall_id) {
    return syscall_handlers[syscall_id];
}

void register_syscall_handler(const uint32_t syscall_id, const syscall_callback_type handler) {
    syscall_handlers[syscall_id] = handler;
}

#ifdef AARCH64
uint64_t syscall3(uint32_t syscall, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    register uint64_t x8 __asm__("x8") = syscall;
    register uint64_t x0 __asm__("x0") = arg0;
    register uint64_t x1 __asm__("x1") = arg1;
    register uint64_t x2 __asm__("x2") = arg2;
    __asm volatile("svc #0" : "+r" (x0) : "r"(x8), "r"(x1), "r"(x2) : "memory");

    return x0;
}
#endif

uint32_t uart_write(const uint32_t syscall_id, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    const char *text = (const char *)arg1;
    uart_puts(text);

    return 0;
}