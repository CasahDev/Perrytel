#include <stdint.h>

#include <drivers/fat32.h>
#include <drivers/sd.h>

#include <kernel/memory.h>

#include "fat32_internal.h"

uint32_t fat32_allocate_cluster() {
    void *buffer_fat = kalloc(FAT_SECTOR_SIZE);
    sd_read_block(get_starting_fat_sector(), buffer_fat);
    uint32_t *fat = buffer_fat;

    uint32_t cluster = 3;
    uint32_t sector = 0;
    while (fat[cluster] != EMPTY_CLUSTER) {
        cluster++;
        if (cluster == 128) {
            sector++;
            cluster = 0;
            sd_read_block(get_starting_fat_sector() + sector, buffer_fat);
            fat = buffer_fat;
        }
    }

    fat[cluster] = CLUSTER_END;
    sd_write_block(get_starting_fat_sector() + sector, buffer_fat);
    kfree(buffer_fat);

    return cluster + sector * 128;
}
