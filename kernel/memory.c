#include <kernel/memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <kernel/string.h>

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
            current->is_free = false;
            return (void*)(current + 1);
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

    struct block_meta *header = (struct block_meta *)ptr - 1;
    header->is_free = true;
}

void memset(void *ptr, int value, size_t size) {
    char *mem = ptr;
    for (size_t i = 0; i < size; i++) {
        mem[i] = value;
    }
}