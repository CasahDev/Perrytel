//
// Created by sacha on 26/08/2026.
//


#include <stdint.h>
#include <kernel/memory.h>
#include <kernel/shell.h>
#include <kernel/syscall.h>
#include <kernel/user.h>

int32_t sys_exit_handler(const uint32_t syscall_id, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    (void)syscall_id;
    (void)arg0;
    (void)arg1;
    (void)arg2;

    kfree(current_user_stack);
    __asm__ volatile("msr daifclr, #2");
    run_shell();

    return 0;
}
