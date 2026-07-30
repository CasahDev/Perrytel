#pragma once
#include <stdbool.h>
#include <stddef.h>

struct block_meta {
    unsigned int size;
    bool is_free;
    struct block_meta *next_block;
};

void* kalloc(unsigned int size); 

void kfree(void *ptr);

void memset(void *ptr, int value, size_t size);

void meminfo(int argc, char **argv);