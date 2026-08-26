#include <string.h>
#include <drivers/uart.h>

#include <drivers/fat32.h>
#include <drivers/sd.h>

#include <kernel/memory.h>

#include "fat32_internal.h"



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


int fat32_delete_entry_raw(fat32_dir_entry *entry, void *directory_buffer) {
    uint32_t sector = get_current_directory_sector();

    entry->name[0] = DELETED;

    sd_write_block(sector, directory_buffer);

    uint32_t cluster = (entry->cluster_entry_high << 16) | entry->cluster_entry_low;

    void *file_buffer = kalloc(512);
    while (cluster < EOF) {
        const uint32_t lba_sector = get_starting_fat_sector() + cluster / 128;
        sd_read_block(lba_sector, file_buffer);

        uint32_t *fat = file_buffer;
        const uint32_t next_cluster = fat[cluster % 128];

        fat[cluster % 128] = EMPTY_CLUSTER;

        sd_write_block(lba_sector, file_buffer);
        cluster = next_cluster;
    }

    kfree(file_buffer);

    return 0;
}

static uint8_t get_file_content(char *out, const fat32_dir_entry *entry, const uint32_t max_size) {
    uint32_t cluster_num = (entry->cluster_entry_high << 16) + entry->cluster_entry_low;
    uint32_t bytes_read = 0;
    void *fat_buffer = kalloc(512);

    void *file_name = kalloc(11);
    fat32_get_name(entry, file_name);

    while (cluster_num < EOF && bytes_read < entry->size) {
        if (bytes_read + 512 > max_size) {
            kfree(fat_buffer);
            kfree(file_name);
            return 2;
        }

        const uint32_t lba_sector = get_starting_data_sector() + (cluster_num - 2) * get_sectors_per_cluster();

        sd_read_block(lba_sector, (uint8_t *) out + bytes_read);
        bytes_read += 512;

        const uint32_t fat_sector = get_starting_fat_sector() + cluster_num / 128;
        sd_read_block(fat_sector, fat_buffer);
        cluster_num = ((uint32_t *) fat_buffer)[cluster_num % 128];
    }

    out[entry->size] = '\0';
    kfree(fat_buffer);

    return 0;
}

static void change_dir_and_extract_filename(char *out, char *path) {
    char given_path[128];
    char *given_ptr = given_path;
    strcpy(given_ptr, path);

    char *split_path[16];
    const int parts = strsplit(given_ptr, split_path, 16, '/');

    for (int i = 0; i < 11; i++) {
        out[i] = split_path[parts - 1][i];
    }

    if (parts > 1) {
        uint32_t c = strlen(path) - 1;

        while (path[c] != '/' && c > 0) {
            c--;
        }

        if (c > 0) {
            path[c] = '\0';
        } else {
            path[0] = '/';
            path[1] = '\0';
        }

        fat32_change_dir(path);

    }
}

// TODO Gérer les fichiers dans des sous dossiers
int fat32_read_file(const char *path, char *content, const uint32_t max_size) {
    int status = 1;
    char orig_path[128];
    get_path(orig_path);

    char file[11];
    change_dir_and_extract_filename(file, path);

    void *buffer = kalloc(512);

    sd_read_block(get_current_directory_sector(), buffer);

    for (int i = 0; i < 16; i++) {
        const fat32_dir_entry *entry = &buffer[i * 32];
        if ((uint8_t) entry->name[0] != EMPTY && (uint8_t) entry->name[0] != DELETED) {
            void *file_name = kalloc(11);
            fat32_get_name(entry, file_name);
            if (strcmp(file_name, file) != 0) {
                status = get_file_content(content, entry, max_size);

                kfree(file_name);
                kfree(buffer);
                break;
            }
            kfree(file_name);
        }
    }

    fat32_change_dir(orig_path);

    return status;
}

int fat32_remove_file(const char *path) {
    void *buffer = kalloc(512);
    sd_read_block(get_current_directory_sector(), buffer);

    for (int i = 0; i < 16; i++) {
        fat32_dir_entry *entry = &buffer[i * 32];
        if ((uint8_t) entry->name[0] == EMPTY) {
            kfree(buffer);
            return 2;
        }

        if ((uint8_t) entry->name[0] == DELETED) continue;

        void *file = kalloc(11);
        fat32_get_name(entry, file);
        if (strcmp(file, path) == 0) {
            kfree(file);
            continue;
        }
        kfree(file);

        if (entry->attributes == DIRECTORY) {
            kfree(buffer);
            return 1;
        }

        fat32_delete_entry_raw(entry, buffer);
        return 0;
    }

    kfree(buffer);
    return 1;
}


int fat32_create_file(const char *file_name, const char *content) {
    const uint32_t global_cluster = fat32_allocate_cluster();
    const uint32_t lba_sector = get_starting_data_sector() + (global_cluster - 2) * get_sectors_per_cluster();

    char *file_content = kalloc(512);
    memset(file_content, 0x00, 512);

    strcpy(file_content, content);

    sd_write_block(lba_sector, (uint8_t *) file_content);

    void *directory_buffer = kalloc(512);
    sd_read_block(get_current_directory_sector(), directory_buffer);

    int slot = 0;
    fat32_dir_entry *entries = (fat32_dir_entry *) directory_buffer;
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

    sd_write_block(get_current_directory_sector(), directory_buffer);

    kfree(directory_buffer);

    kfree(file_content);

    return 0;
}