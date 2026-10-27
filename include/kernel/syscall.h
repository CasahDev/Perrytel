#pragma once


typedef int32_t (*syscall_callback_type) (uint32_t, uint64_t, uint64_t, uint64_t);

void *el0_sync_handler(void *sp);
void init_syscall_handler(void);
void register_syscall_handler(const uint32_t syscall_id, const syscall_callback_type handler);
syscall_callback_type get_syscall_handler(uint32_t syscall_id);

int32_t uart_write(const uint32_t syscall_id, uint64_t arg0, uint64_t arg1, uint64_t arg2);

int32_t sys_exit_handler(const uint32_t syscall_id, uint64_t arg0, uint64_t arg1, uint64_t arg2);
uint32_t uart_read(const uint32_t syscall_id, uint64_t arg0, uint64_t arg1, uint64_t arg2);

int32_t sys_open(const uint32_t syscall_id, uint64_t arg0, uint64_t arg1, uint64_t arg2);

int32_t sys_read(const uint32_t syscall_id, uint64_t arg0, uint64_t arg1, uint64_t arg2);
int32_t sys_close(const uint32_t syscall_id, uint64_t arg0, uint64_t arg1, uint64_t arg2);

void init_fd();