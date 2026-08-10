//
// Created by sacha on 30/07/2026.
//

#include <perrylib.h>

#include <kernel/memory.h>
#include <kernel/user.h>

void user_mode() {
    void *user_stack = kalloc(4096);

    void *stack_top = user_stack + 4096;

    switch_to_user_mode(user_main_entry, stack_top);
}

__attribute__((section(".user_text"))) void user_main_entry() {
    printf("Perrytel OS (AArch64) - Kernel Loaded\r\n");

    printf("Perry -> ");
    while(1) {
        __asm volatile("wfe");
    }
}