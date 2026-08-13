#include <string.h>

#include <drivers/fat32.h>
#include <drivers/sd.h>

#include <kernel/memory.h>

static uint32_t starting_partition_sector;
static uint32_t starting_fat_sector;
static uint32_t starting_data_sector;
static uint32_t sectors_per_cluster;
static uint32_t root_sector;
static uint32_t current_directory_cluster = 2;

void fat32_init(const uint32_t starting_sector, const uint8_t *cluster) {
    const fat32_boot_sector *sector = (const fat32_boot_sector *) cluster;

    starting_partition_sector = starting_sector;
    starting_fat_sector = starting_partition_sector + sector->reserved_sector_number;
    starting_data_sector = starting_fat_sector + sector->sectors_per_fat * sector->fats_number;
    sectors_per_cluster = sector->sectors_per_cluster;
    root_sector = starting_data_sector + (sector->root_directory_cluster - 2) * sectors_per_cluster;
}

// region Getters/Setters

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

uint32_t get_current_directory() {
    return current_directory_cluster;
}

void set_current_directory(const uint32_t new_directory) {
    current_directory_cluster = new_directory;
}

uint32_t get_current_directory_sector() {
    return starting_data_sector + (get_current_directory() - 2) * sectors_per_cluster;
}

// endregion

// region Drivers
char *fat32_read_file(const fat32_dir_entry *entry) {
    char *buffer = kalloc(entry->size + 1);

    uint32_t cluster_num = (entry->cluster_entry_high << 16) + entry->cluster_entry_low;
    uint32_t bytes_read = 0;
    void *fat_buffer = kalloc(512);

    while (cluster_num < EOF && bytes_read < entry->size) {
        const uint32_t lba_sector = starting_data_sector + (cluster_num - 2) * sectors_per_cluster;

        sd_read_block(lba_sector, (uint8_t *) buffer + bytes_read);
        bytes_read += 512;

        const uint32_t fat_sector = starting_fat_sector + cluster_num / 128;
        sd_read_block(fat_sector, fat_buffer);
        cluster_num = ((uint32_t *) fat_buffer)[cluster_num % 128];
    }

    kfree(fat_buffer);
    buffer[entry->size] = '\0';
    return buffer;
}

uint32_t fat32_allocate_cluster() {
    void *buffer_fat = kalloc(512);
    sd_read_block(starting_fat_sector, buffer_fat);
    uint32_t *fat = buffer_fat;

    uint32_t cluster = 3;
    uint32_t sector = 0;
    while (fat[cluster] != EMPTY_CLUSTER) {
        cluster++;
        if (cluster == 128) {
            sector++;
            cluster = 0;
            sd_read_block(starting_fat_sector + sector, buffer_fat);
            fat = buffer_fat;
        }
    }

    fat[cluster] = CLUSTER_END;
    sd_write_block(starting_fat_sector + sector, buffer_fat);
    kfree(buffer_fat);

    return cluster + sector * 128;
}

void fat32_get_name(const fat32_dir_entry *entry, char *out_name) {
    uint8_t char_count = 0;
    for (int i = 0; i < 8; i++) {
        if (entry->name[i] != ' ') {
            out_name[char_count] = entry->name[i];
            char_count++;
        }
    }

    if (entry->name[8] != ' ' || entry->name[9] != ' ' || entry->name[10] != ' ') {
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
    const uint32_t global_cluster = fat32_allocate_cluster();
    const uint32_t lba_sector = starting_data_sector + (global_cluster - 2) * sectors_per_cluster;

    char *file_content = kalloc(512);

    for (int i = 0; i < 512; i++) {
        file_content[i] = 0x00;
    }

    strcpy(file_content, content);

    sd_write_block(lba_sector, (uint8_t *) file_content);

    void *root_buffer = kalloc(512);
    sd_read_block(get_current_directory_sector(), root_buffer);

    int slot = 0;
    fat32_dir_entry *entries = (fat32_dir_entry *) root_buffer;
    while (slot < 16 && (uint8_t) entries[slot].name[0] != 0x00 && (uint8_t) entries[slot].name[0] != DELETED) {
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
    entry->attributes = ARCHIVE;

    entry->cluster_entry_high = global_cluster >> 16;
    entry->cluster_entry_low = global_cluster & 0xFFFF;
    entry->size = strlen(content);

    sd_write_block(root_sector, root_buffer);

    kfree(root_buffer);

    kfree(file_content);
}

void fat32_create_directory(const char *directory) {
    const uint32_t global_cluster = fat32_allocate_cluster();
    const uint32_t lba_sector = starting_data_sector + (global_cluster - 2) * sectors_per_cluster;

    // Reset du contenu de la carte a l'emplacement
    void *dir_content = kalloc(512);
    memset(dir_content, 0, 512);

    // Création des fichiers . et ..

    fat32_dir_entry *entries = (fat32_dir_entry *) dir_content;
    fat32_dir_entry *new_dir = &entries[0];
    new_dir->name[0] = '.';

    for (int i = 0; i < 10; i++) {
        new_dir->name[i + 1] = ' ';
    }

    new_dir->attributes = DIRECTORY;
    new_dir->cluster_entry_high = global_cluster >> 16;
    new_dir->cluster_entry_low = global_cluster & 0xFFFF;

    fat32_dir_entry *parent_dir = &entries[1];
    parent_dir->name[0] = '.';
    parent_dir->name[1] = '.';

    for (int i = 0; i < 9; i++) {
        parent_dir->name[i + 2] = ' ';
    }

    parent_dir->attributes = DIRECTORY;

    if (current_directory_cluster == 2) {
        parent_dir->cluster_entry_high = 0;
        parent_dir->cluster_entry_low = 0;
    } else {
        parent_dir->cluster_entry_high = current_directory_cluster >> 16;
        parent_dir->cluster_entry_low = current_directory_cluster & 0xFFFF;
    }

    sd_write_block(lba_sector, dir_content);

    kfree(dir_content);

    void *parent_directory = kalloc(512);
    uint32_t parent_sector = starting_data_sector + (current_directory_cluster - 2) * sectors_per_cluster;
    sd_read_block(parent_sector, parent_directory);

    fat32_dir_entry *parent_entries = parent_directory;
    int slot = 0;
    while (slot < 16 && (uint8_t) parent_entries[slot].name[0] != 0x00 && (uint8_t) parent_entries[slot].name[0] !=
           DELETED) {
        slot++;
    }

    fat32_dir_entry *entry = &parent_entries[slot];
    memset(entry->name, ' ', 11);

    entry->attributes = DIRECTORY;
    entry->cluster_entry_high = global_cluster >> 16;
    entry->cluster_entry_low = global_cluster & 0xFFFF;
    entry->size = 0;

    uint8_t max = strlen(directory) > 11 ? 11 : strlen(directory);

    for (int i = 0; i < max; i++) {
        entry->name[i] = directory[i];
    }

    sd_write_block(parent_sector, parent_directory);
    kfree(parent_directory);
}

void fat32_remove_file(fat32_dir_entry *entry, void *parent_directory, uint32_t sector) {
    if (sector == 0) sector = get_current_directory_sector();

    entry->name[0] = DELETED;
    sd_write_block(sector, parent_directory);

    uint32_t cluster = (entry->cluster_entry_high << 16) | entry->cluster_entry_low;

    void *buffer = kalloc(512);
    while (cluster < EOF) {
        const uint32_t lba_sector = starting_fat_sector + cluster / 128;
        sd_read_block(lba_sector, buffer);

        uint32_t *fat = buffer;
        const uint32_t next_cluster = fat[cluster % 128];

        fat[cluster % 128] = EMPTY_CLUSTER;

        sd_write_block(lba_sector, buffer);
        cluster = next_cluster;
    }

    kfree(buffer);
}


void fat32_remove_directory(fat32_dir_entry *entry, void *parent_directory, uint32_t parent_sector) {
    if (parent_sector == 0) parent_sector = get_current_directory_sector();
    uint32_t cluster = (entry->cluster_entry_high << 16) | entry->cluster_entry_low;

    void *buffer = kalloc(512);
    const uint32_t   lba_sector = starting_data_sector + (cluster - 2) * sectors_per_cluster;
    sd_read_block(lba_sector, buffer);
    fat32_dir_entry *entries = buffer;

    for (int i = 0; i < 16; i++) {
        if (entries[i].name[0] == EMPTY) break;

        if (entries[i].name[0] == DELETED) continue;

        if (entries[i].name[0] == '.' && (entries[i].name[1] == ' ' || entries[i].name[1] == '.')) continue;

        if (entries[i].attributes == DIRECTORY) {
            fat32_remove_directory(&entries[i], buffer, lba_sector);
        } else {
            fat32_remove_file(&entries[i], buffer, lba_sector);
        }
    }

    fat32_remove_file(entry, parent_directory, parent_sector);
    kfree(buffer);
}

// endregion