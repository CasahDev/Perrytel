//
// Created by sacha on 26/08/2026.
//


#include <stdint.h>
#include <kernel/syscall.h>

#include <kernel/shell.h>

uint32_t sys_exit_handler(const uint32_t syscall_id, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    print_shell();
    return 0;
}
