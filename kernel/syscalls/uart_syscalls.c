//
// Created by sacha on 26/08/2026.
//

#include <stdint.h>

#include <drivers/uart.h>

#include <kernel/syscall.h>


uint32_t uart_read(const uint32_t syscall_id, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    char *input = (char *) arg1;
    uint64_t count = arg2;
    uint32_t bytes_read = 0;

    while (bytes_read < count) {
        int c;
        while ((c = uart_get_char_async()) == -1) {
            __asm__ volatile("msr daifclr, #2");
            __asm volatile("wfe");
        }

        input[bytes_read] = (char) c;
        bytes_read++;

        if (c == '\r' || c == '\n') break;
    }

    return bytes_read;
}


int32_t uart_write(const uint32_t syscall_id, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    const char *text = (const char *) arg1;
    uart_puts(text);

    return 0;
}