#include <stdint.h>
#include <stdbool.h>

extern uint8_t __end;

struct block_meta {
    unsigned int size;
    bool is_free;
    struct block_meta *next_block;
};

void* kalloc(unsigned int size); 

void kfree(void *ptr);

void memset(void *ptr, int value, size_t size);