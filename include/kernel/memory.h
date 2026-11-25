#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    KERNEL_BASE_ADDRESS = 0x80000,

    // Base heap address for dynamic allocation
    HEAP_START_ADDRESS = 0x300000,

    // Memory address to load user binaries
    USER_PROGRAM_BASE = 0x200000,

    // Page size on a frame
    PAGE_SIZE = 4096,

    // Binary max size
    USER_BIN_MAX_SIZE = 64000
};

struct block_meta {
    unsigned int size;
    bool is_free;
    struct block_meta *next_block;
};

void *kalloc(unsigned int size);

void kfree(void *ptr);

void memset(void *ptr, int value, size_t size);

void meminfo(int argc, char **argv);

char *memmove(void *dest, const void *src, uint64_t len);

char *memcpy(void *dest, const void *src, unsigned long size);

int memcmp(const void *s1, const void *s2, unsigned long￹size);
