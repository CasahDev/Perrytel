#include <drivers/fat32.h>
#include <drivers/sd.h>

#include <kernel/memory.h>

static uint32_t starting_partition_sector;
static uint32_t starting_fat_sector;
static uint32_t starting_data_sector;
static uint32_t sectors_per_cluster;
static uint32_t root_sector;

void fat32_init(const uint32_t starting_sector, const uint8_t *cluster) {
    const fat32_boot_sector *sector = (const fat32_boot_sector *) cluster;

    starting_partition_sector = starting_sector;
    starting_fat_sector = starting_partition_sector + sector->reserved_sector_number;
    starting_data_sector = starting_fat_sector + sector->sectors_per_fat * sector->fats_number;
    sectors_per_cluster = sector->sectors_per_cluster;
    root_sector = starting_data_sector + (sector->root_directory_cluster - 2) * sectors_per_cluster;
}

char *fat32_read_file(const fat32_dir_entry *entry) {
    void *buffer = kalloc(entry->size + 1);

    const uint32_t cluster_num = (entry->cluster_entry_high << 16) + entry->cluster_entry_low;
    const uint32_t lba_sector = starting_data_sector + (cluster_num - 2) * sectors_per_cluster;

    sd_read_block(lba_sector, buffer);

    return buffer;
}