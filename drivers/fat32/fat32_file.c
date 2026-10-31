#include <string.h>

#include <drivers/fat32.h>
#include <drivers/sd.h>

#include <kernel/memory.h>

#include "fat32_internal.h"


void fat32_get_name(const fat32_dir_entry *entry, char *out_name) {
    uint8_t char_count = 0;
    for (int i = 0; i < FAT_NAME_CHARS; i++) {
        if (entry->name[i] != ' ') {
            out_name[char_count] = to_lower(entry->name[i]);
            char_count++;
        }
    }

    if (entry->name[FAT_NAME_CHARS] != ' ' || entry->name[FAT_NAME_CHARS + 1] != ' ' || entry->name[FAT_NAME_CHARS + 2]
        != ' ') {
        out_name[char_count] = '.';
        char_count++;
        for (int i = 0; i < FAT_EXT_CHARS; i++) {
            if (entry->name[FAT_NAME_CHARS + i] != ' ') {
                out_name[char_count] = to_lower(entry->name[FAT_NAME_CHARS + i]);
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

    void *file_buffer = kalloc(FAT_SECTOR_SIZE);
    while (cluster < EOF) {
        const uint32_t lba_sector = get_starting_fat_sector() + cluster / FAT_ENTRIES_PER_SECTOR;
        sd_read_block(lba_sector, file_buffer);

        uint32_t *fat = file_buffer;
        const uint32_t next_cluster = fat[cluster % FAT_ENTRIES_PER_SECTOR];

        fat[cluster % FAT_ENTRIES_PER_SECTOR] = EMPTY_CLUSTER;

        sd_write_block(lba_sector, file_buffer);
        cluster = next_cluster;
    }

    kfree(file_buffer);

    return 0;
}

static uint8_t get_file_content(char *out, const fat32_dir_entry *entry, const uint32_t max_size) {
    uint32_t cluster_num = (entry->cluster_entry_high << 16) + entry->cluster_entry_low;
    uint32_t bytes_read = 0;
    void *fat_buffer = kalloc(FAT_SECTOR_SIZE);

    while (cluster_num < EOF && bytes_read < entry->size) {
        if (bytes_read + FAT_SECTOR_SIZE > max_size) {
            kfree(fat_buffer);
            return 2;
        }

        const uint32_t lba_sector = get_starting_data_sector() + (cluster_num - 2) * get_sectors_per_cluster();

        sd_read_block(lba_sector, (uint8_t *) out + bytes_read);
        bytes_read += FAT_SECTOR_SIZE;

        const uint32_t fat_sector = get_starting_fat_sector() + cluster_num / 128;
        sd_read_block(fat_sector, fat_buffer);
        cluster_num = ((uint32_t *) fat_buffer)[cluster_num % 128];
    }

    out[entry->size] = '\0';
    kfree(fat_buffer);

    return 0;
}

static void change_dir_and_extract_filename(char *out, const char *path) {
    char given_path[128];
    char *given_ptr = given_path;
    strcpy(given_ptr, path);

    if (path[0] == '/') {
        fat32_change_dir("/");
    }

    char *split_path[16];
    const int parts = strsplit(given_ptr, split_path, 16, '/');

    for (unsigned long i = 0; i < strlen(split_path[parts - 1]); i++) {
        out[i] = split_path[parts - 1][i];
    }

    if (parts > 1) {
        for (int i = 0; i < parts - 1; i++) {
            fat32_change_dir(split_path[i]);
        }
    }
}

// TODO Gérer les fichiers dans des sous dossiers
int fat32_read_file(const char *path, char *content, const uint32_t max_size) {
    int status = 1;
    char orig_path[128];
    get_path(orig_path);

    char file[FAT_SHORT_NAME_LEN];
    change_dir_and_extract_filename(file, path);

    uint8_t *buffer = kalloc(FAT_SECTOR_SIZE);

    sd_read_block(get_current_directory_sector(), buffer);

    for (int i = 0; i < FAT_DIR_ENTRIES_PER_SECTOR; i++) {
        const fat32_dir_entry *entry = (const fat32_dir_entry *) &buffer[i * FAT_DIR_ENTRY_SIZE];
        if ((uint8_t) entry->name[0] != EMPTY && (uint8_t) entry->name[0] != DELETED) {
            void *file_name = kalloc(FAT_SHORT_NAME_LEN);
            fat32_get_name(entry, file_name);
            if (streq(file_name, file) != 0) {
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
    uint8_t *buffer = kalloc(FAT_SECTOR_SIZE);
    sd_read_block(get_current_directory_sector(), buffer);

    for (int i = 0; i < FAT_DIR_ENTRIES_PER_SECTOR; i++) {
        fat32_dir_entry *entry = (fat32_dir_entry *) &buffer[i * FAT_DIR_ENTRY_SIZE];
        if ((uint8_t) entry->name[0] == EMPTY) {
            kfree(buffer);
            return 2;
        }

        if ((uint8_t) entry->name[0] == DELETED) continue;

        void *file = kalloc(FAT_SHORT_NAME_LEN);
        fat32_get_name(entry, file);
        if (streq(file, path) == 0) {
            kfree(file);
            kfree(buffer);
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

    char *file_content = kalloc(FAT_SECTOR_SIZE);
    memset(file_content, 0x00, FAT_SECTOR_SIZE);

    strcpy(file_content, content);

    sd_write_block(lba_sector, (uint8_t *) file_content);

    void *directory_buffer = kalloc(FAT_SECTOR_SIZE);
    sd_read_block(get_current_directory_sector(), directory_buffer);

    int slot = 0;
    fat32_dir_entry *entries = (fat32_dir_entry *) directory_buffer;
    while (slot < 16 && (uint8_t) entries[slot].name[0] != 0x00 && (uint8_t) entries[slot].name[0] != DELETED) {
        slot++;
    }

    fat32_dir_entry *entry = &entries[slot];

    memset(entry->name, ' ', FAT_SHORT_NAME_LEN);

    uint32_t i = 0;
    while (i < FAT_NAME_CHARS && file_name[i] != '\0' && file_name[i] != '.') {
        entry->name[i] = file_name[i];
        i++;
    }

    if (file_name[i] == '.') i++;

    if (strlen(file_name) > (unsigned long) i) {
        for (int j = 0; j < FAT_EXT_CHARS; j++) {
            entry->name[FAT_NAME_CHARS + j] = file_name[i];
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

bool fat32_file_exists(const char *path) {
    char orig_path[128];
    get_path(orig_path);

    bool exists = false;

    char filename[FAT_SHORT_NAME_LEN];
    change_dir_and_extract_filename(filename, path);

    uint8_t *buffer = kalloc(FAT_SECTOR_SIZE);
    sd_read_block(get_current_directory_sector(), buffer);
    for (int i = 0; i < FAT_DIR_ENTRIES_PER_SECTOR; i++) {
        const fat32_dir_entry *entry = (const fat32_dir_entry *) &buffer[i * FAT_DIR_ENTRY_SIZE];
        if ((uint8_t) entry->name[0] == EMPTY || (uint8_t) entry->name[0] == DELETED) continue;

        void *file_name = kalloc(FAT_SHORT_NAME_LEN);
        fat32_get_name(entry, file_name);

        if (streq(file_name, filename) == 0) {
            kfree(file_name);
            continue;
        }

        kfree(file_name);
        exists = true;
        break;
    }

    fat32_change_dir(orig_path);
    kfree(buffer);
    return exists;
}

void fat32_open(const char *path, vfs_file *out_file) {
    char orig_path[128];
    get_path(orig_path);

    char filename[FAT_SHORT_NAME_LEN];
    change_dir_and_extract_filename(filename, path);


    uint8_t *buffer = kalloc(FAT_SECTOR_SIZE);
    sd_read_block(get_current_directory_sector(), buffer);
    for (int i = 0; i < FAT_DIR_ENTRIES_PER_SECTOR; i++) {
        const fat32_dir_entry *entry = (const fat32_dir_entry *) &buffer[i * FAT_DIR_ENTRY_SIZE];
        if ((uint8_t) entry->name[0] == EMPTY || (uint8_t) entry->name[0] == DELETED) continue;

        void *file_name = kalloc(FAT_SHORT_NAME_LEN);
        fat32_get_name(entry, file_name);

        if (streq(file_name, filename) == 0) {
            kfree(file_name);
            continue;
        }

        out_file->driver = &fat32_driver;
        out_file->size = entry->size;
        out_file->private_id = entry->cluster_entry_high << 16 | entry->cluster_entry_low;

        kfree(file_name);
        break;
    }
    kfree(buffer);
}

uint64_t fat32_read(vfs_file *file, char *buffer, const uint64_t count) {
    const uint64_t cluster = file->private_id;
    const uint64_t base_sector = get_starting_data_sector() + (cluster - 2) * get_sectors_per_cluster();

    uint8_t *temp = kalloc(FAT_SECTOR_SIZE);

    uint32_t bytes_read = 0;
    const uint16_t sector_offset = file->offset % FAT_SECTOR_SIZE;
    while (file->offset < file->size && bytes_read < count) {
        const uint64_t remaining = file->size - file->offset;

        const uint64_t lba_sector = base_sector + (file->offset / FAT_SECTOR_SIZE);
        sd_read_block(lba_sector, temp);

        uint32_t chunk;
        if (remaining > (uint64_t) FAT_SECTOR_SIZE - sector_offset) {
            chunk = FAT_SECTOR_SIZE - sector_offset;
        } else {
            chunk = remaining;
        }

        if (chunk > count - bytes_read) {
            chunk = count - bytes_read;
        }

        for (uint32_t i = 0; i < chunk; i++) {
            buffer[bytes_read + i] = (char) temp[sector_offset + i];
        }

        file->offset += chunk;
        bytes_read += chunk;
    }

    kfree(temp);
    return bytes_read;
}
