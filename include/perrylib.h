#pragma once
#include <stdint.h>
#include <errors.h>

/**
 * SYS CALL Numbers
 */
#define SYS_READ 63
#define SYS_WRITE 64
#define SYS_EXIT 93
#define SYS_OPEN 56
#define SYS_CLOSE 57

uint64_t syscall1(uint32_t syscall, uint64_t arg0);
uint64_t syscall2(uint32_t syscall, uint64_t arg0, uint64_t arg1);
uint64_t syscall3(uint32_t syscall, uint64_t arg0, uint64_t arg1, uint64_t arg2);

int64_t readf(int fd, char *out, uint64_t buffer_size);
void printf(char *str);
void exit(uint32_t code);

int64_t open(char *file);
int64_t close(uint64_t fd);