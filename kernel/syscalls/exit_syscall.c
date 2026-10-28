//
// Created by sacha on 26/08/2026.
//


#include <stdint.h>
#include <kernel/memory.h>
#include <kernel/shell.h>
#include <kernel/syscall.h>
#include <kernel/user.h>

int32_t sys_exit_handler(const uint32_t syscall_id, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    kfree(current_user_stack);
    print_shell();
    return 0;
}
