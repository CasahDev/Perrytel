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

uint32_t get_starting_partition_sector() {
    return starting_data_sector;
}

uint32_t get_starting_fat_sector() {
    return starting_fat_sector;
}

uint32_t get_starting_data_sector() {
    return starting_data_sector;
}

uint32_t get_sectors_per_cluster() {
    return sectors_per_cluster;
}

uint32_t get_root_sector() {
    return root_sector;
}

void fat32_get_name(const fat32_dir_entry *entry, char *out_name) {
    uint8_t char_count = 0;
    for (int i = 0; i < 8; i++) {
        if (entry->name[i] != ' ') {
            out_name[char_count] = entry->name[i];
            char_count++;
        }
    }

    if (entry->name[8] != 0x00 || entry->name[9] != 0x00 || entry->name[10] != 0x00) {
        out_name[char_count] = '.';
        char_count++;
        for (int i = 0; i < 3; i++) {
            if (entry->name[8 + i] != ' ') {
                out_name[char_count] = entry->name[8 + i];
                char_count++;
            }
        }
    }
    out_name[char_count] = '\0';
}