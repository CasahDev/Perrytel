#pragma once
#include <stdint.h>

#define SYS_READ 63
#define SYS_WRITE 64
#define EXIT_SYSCALL 93

uint64_t syscall1(uint32_t syscall, uint64_t arg0);
uint64_t syscall2(uint32_t syscall, uint64_t arg0, uint64_t arg1);
uint64_t syscall3(uint32_t syscall, uint64_t arg0, uint64_t arg1, uint64_t arg2);
void printf(char *str);
void exit(uint32_t code);