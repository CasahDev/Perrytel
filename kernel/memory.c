#include <kernel/memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <kernel/string.h>
#include <drivers/uart.h>

struct block_meta *heap_start = NULL; 


void *kalloc(unsigned int size) {
    if (heap_start == NULL) {
        // On pose notre calque sur l'adresse de __end
        heap_start = (struct block_meta *)&__end; 
        
        // On remplit les informations du Header
        heap_start->size = size;
        heap_start->is_free = false;
        heap_start->next_block = NULL; // Il n'y a personne après nous pour le moment     

        // On retourne l'adresse de la mémoire UTILISABLE (juste après le Header)         
        // L'astuce (heap_start + 1) en C avance l'adresse de la taille exacte du Header !
        return (void*)(heap_start + 1);
    }

    struct block_meta *current = heap_start;
    struct block_meta *last = heap_start; // Va retenir le dernier bloc valide
    
    while (current != NULL)
    {
        if (current->is_free && current->size > size) {
            if (current->size > size + sizeof(struct block_meta) + 20) {
                struct block_meta *next = (struct block_meta*)((uint8_t *) current + sizeof(struct block_meta) + size);
                next->size = current->size - (size + sizeof(struct block_meta));
                next->is_free = true;
                next->next_block = current->next_block;

                current->next_block = next;
                current->size = size;
            }
            current->is_free = false;
            return current + 1;
        }

        last = current;
        current = current->next_block;
    }

    // (uint8_t*) permet de calculer en "octets"
    struct block_meta *new_block = (struct block_meta *)((uint8_t*)(last + 1) + last->size);
    new_block->size = size;
    new_block->is_free = false;
    new_block->next_block = NULL;

    last->next_block = new_block;
    return (void*)(new_block + 1);
}

void kfree(void *ptr) {
    if (ptr == NULL) return;

    const struct block_meta *header = (struct block_meta *)ptr - 1;

    struct block_meta *new_header = (struct block_meta *)ptr - 1;
    new_header->is_free = true;
    while (new_header->next_block != NULL && new_header->next_block->is_free) {
        new_header->size = header->size + header->next_block->size + sizeof(struct block_meta);
        new_header->next_block = header->next_block->next_block;
    }
}

void memset(void *ptr, const int value, const size_t size) {
    char *mem = ptr;
    for (size_t i = 0; i < size; i++) {
        mem[i] = value;
    }
}

void meminfo(int argc, char **argv) {
    int blocks = 0;
    int free = 0;
    int occupied = 0;
    int allocated_size = 0;
    int free_size = 0;

    const struct block_meta *current = heap_start;
    while (current != NULL) {
        blocks++;
        if (current->is_free) {
            free_size += current->size;
            free++;
        } else {
            allocated_size += current->size;
            occupied++;
        }

        current = current->next_block;
    }

    char b[15];
    itoa(blocks, b);

    char f[15];
    itoa(free, f);

    char o[15];
    itoa(occupied, o);

    char as[15];
    itoa(allocated_size, as);

    char fs[15];
    itoa(free_size, fs);

    uart_puts("Blocks: ");
    uart_puts(b);
    uart_puts("\r\n");

    uart_puts("Free: ");
    uart_puts(f);
    uart_puts("\r\n");

    uart_puts("Occupied: ");
    uart_puts(o);
    uart_puts("\r\n");

    uart_puts("Allocated memory: ");
    uart_puts(as);
    uart_puts("\r\n");

    uart_puts("Free memory: ");
    uart_puts(fs);
    uart_puts("\r\n");
}