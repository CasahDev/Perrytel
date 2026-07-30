//
// Created by sacha on 30/07/2026.
//

#include <kernel/user.h>
#include <kernel/memory.h>

void user_mode() {
    void *user_stack = kalloc(4096);

    void *stack_top = user_stack + 4096;

    switch_to_user_mode(user_main_entry, stack_top);
}

void user_main_entry() {
    while(1) {

    }
}