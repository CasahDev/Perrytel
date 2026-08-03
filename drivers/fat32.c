#include <kernel/string.h>

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

void fat32_create_file(char *file_name, char *content) {
    void *buffer_fat = kalloc(512);
    sd_read_block(starting_fat_sector, buffer_fat);
    uint32_t *fat = buffer_fat;

    uint8_t cluster = 3;
    uint8_t sector = 0;
    while (fat[cluster] != 0x00000000) {
        cluster++;
        if (cluster == 128) {
            sector++;
            cluster = 0;
            sd_read_block(starting_fat_sector + sector, buffer_fat);
            fat = buffer_fat;
        }
    }

    fat[cluster] = 0x0FFFFFFF;
    sd_write_block(starting_fat_sector + sector, buffer_fat);

    const uint32_t global_cluster = sector * 128 + cluster;
    const uint32_t lba_sector = starting_data_sector + (global_cluster - 2) * sectors_per_cluster;

    char *file_content = kalloc(512);

    for (int i = 0; i < 512; i++) {
        file_content[i] = 0x00;
    }

    strcpy(file_content, content);

    sd_write_block(lba_sector, (uint8_t *) file_content);

    void *root_buffer = kalloc(512);
    sd_read_block(root_sector, root_buffer);

    int slot = 0;
    fat32_dir_entry *entries = (fat32_dir_entry *) root_buffer;
    while (slot < 16 && (uint8_t) entries[slot].name[0] != 0x00 && (uint8_t) entries[slot].name[0] != 0xE5) {
        slot++;
    }

    fat32_dir_entry *entry = &entries[slot];

    memset(entry->name, ' ', 11);

    int i = 0;
    while (i < 8 && file_name[i] != '\0' && file_name[i] != '.') {
        entry->name[i] = file_name[i];
        i++;
    }

    if (file_name[i] == '.') i++;

    if (strlen(file_name) > i) {
        for (int j = 0; j < 3; j++) {
            entry->name[8 + j] = file_name[i];
            i++;
        }
    }

    // Archive classique (fichier)
    entry->attributes = 0x20;

    entry->cluster_entry_high = global_cluster >> 16;
    entry->cluster_entry_low = global_cluster & 0xFFFF;
    entry->size = strlen(content);

    sd_write_block(root_sector, root_buffer);

    kfree(root_buffer);

    kfree(file_content);

    kfree(buffer_fat);
}
