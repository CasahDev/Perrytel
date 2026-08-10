#pragma once

void user_mode();

void switch_to_user_mode(void *entry_point, void *user_stack);

void user_main_entry();