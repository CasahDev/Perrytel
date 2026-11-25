//
// Created by sacha on 10/08/2026.
//
#include <perrylib.h>
#include <kernel/syscall.h>


static uint64_t syscall1(uint32_t syscall, uint64_t arg0) {
#ifdef AARCH64
    register uint64_t x8 __asm__("x8") = syscall;
    register uint64_t x0 __asm__("x0") = arg0;
    __asm volatile("svc #0" : "+r" (x0) : "r"(x8) : "memory");

    return x0;
#endif
}

uint64_t syscall2(uint32_t syscall, uint64_t arg0, uint64_t arg1) {
#ifdef AARCH64
    register uint64_t x8 __asm__("x8") = syscall;
    register uint64_t x0 __asm__("x0") = arg0;
    register uint64_t x1 __asm__("x1") = arg1;
    __asm volatile("svc #0" : "+r" (x0) : "r"(x8), "r"(x1) : "memory");

    return x0;
#endif
}

uint64_t syscall3(uint32_t syscall, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
#ifdef AARCH64
    register uint64_t x8 __asm__("x8") = syscall;
    register uint64_t x0 __asm__("x0") = arg0;
    register uint64_t x1 __asm__("x1") = arg1;
    register uint64_t x2 __asm__("x2") = arg2;
    __asm volatile("svc #0" : "+r" (x0) : "r"(x8), "r"(x1), "r"(x2) : "memory");

    return x0;
#endif
}

int64_t readf(int fd, char *out, uint64_t buffer_size) {
    return (int32_t) syscall3(SYS_READ, fd, (uint64_t) out, buffer_size);
}

void printf(char *str) {
    syscall2(SYS_WRITE, 1, (uint64_t) str);
}

void exit(const uint32_t code) {
    syscall1(SYS_EXIT, code);
}

int64_t open(char *file) {
    return (int32_t) syscall2(SYS_OPEN, (uint64_t) file, 3);
}

int64_t close(const uint64_t fd) {
    return (int32_t) syscall2(SYS_CLOSE, fd, 0);
}