//
// Created by sacha on 10/08/2026.
//
#include <perrylib.h>


#ifdef AARCH64
uint64_t syscall1(uint32_t syscall, uint64_t arg0) {
    register uint64_t x8 __asm__("x8") = syscall;
    register uint64_t x0 __asm__("x0") = arg0;
    __asm volatile("svc #0" : "+r" (x0) : "r"(x8) : "memory");

    return x0;
}

uint64_t syscall2(uint32_t syscall, uint64_t arg0, uint64_t arg1) {
    register uint64_t x8 __asm__("x8") = syscall;
    register uint64_t x0 __asm__("x0") = arg0;
    register uint64_t x1 __asm__("x1") = arg1;
    __asm volatile("svc #0" : "+r" (x0) : "r"(x8), "r"(x1) : "memory");

    return x0;
}

uint64_t syscall3(uint32_t syscall, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    register uint64_t x8 __asm__("x8") = syscall;
    register uint64_t x0 __asm__("x0") = arg0;
    register uint64_t x1 __asm__("x1") = arg1;
    register uint64_t x2 __asm__("x2") = arg2;
    __asm volatile("svc #0" : "+r" (x0) : "r"(x8), "r"(x1), "r"(x2) : "memory");

    return x0;
}
#endif

void printf(char *str) {
    syscall2(SYS_WRITE, 1, (uint64_t) str);
}

void exit(const uint32_t code) {
    syscall1(EXIT_SYSCALL, code);
}
