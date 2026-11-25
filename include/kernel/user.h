#pragma once
extern void *current_user_stack;

void switch_to_user_mode(void *entry_point, void *user_stack);