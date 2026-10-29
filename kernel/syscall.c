//
// Created by sacha on 04/08/2026.
//

#include "./syscalls/syscall_internal.h"

#include <stdint.h>

#include <drivers/uart.h>

#include <kernel/syscall.h>

syscall_callback_type syscall_handlers[1024];

static int32_t default_handler(const uint32_t syscall_id, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
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