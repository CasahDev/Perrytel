#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
