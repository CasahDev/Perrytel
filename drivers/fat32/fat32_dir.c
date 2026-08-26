
#include <string.h>

#include <drivers/fat32.h>
#include <drivers/sd.h>

#include <kernel/memory.h>

#include "fat32_internal.h"

static char current_path[128] = "/";

int get_path(char *out_path) {
    memset(out_path, ' ', 128);
    const uint32_t len = strlen(current_path);

    uint32_t i = 0;
    while (i < len) {
        out_path[i] = current_path[i];
        i++;
    }

    out_path[i] = '\0';

    return 0;
}


static void go_to_root_dir() {
    set_current_directory(2);
    memset(current_path, 0x00, 128);
    current_path[0] = '/';
    current_path[1] = '\0';
}

static void go_upper_directory() {
    uint8_t c = strlen(current_path) - 1;
    while (current_path[c] != '/' && c > 0) {
        c--;
    }

    if (c == 0) {
        current_path[0] = '/';
        current_path[1] = '\0';
    } else {
        current_path[c] = '\0';
    }
}

static void add_slash_prefix() {
    if (current_path[strlen(current_path) - 1] != '/') {
        current_path[strlen(current_path)] = '/';
        current_path[strlen(current_path)] = '\0';
    }
}

static void add_path(const char *path) {
    add_slash_prefix();

    uint32_t current_pos = strlen(current_path);
    for (uint16_t l = 0; l < strlen(path); l++) {
        current_path[current_pos] = path[l];
        current_pos++;
    }
    current_path[current_pos] = '\0';
}

static int change_directory_depth(const char *path) {
    void *buffer = kalloc(512);
    sd_read_block(get_current_directory_sector(), buffer);
    uint8_t status = 0;

    for (int i = 0; i < 16; i++) {
        const fat32_dir_entry *entry = &buffer[i * 32];

        if ((uint8_t) entry->name[0] == EMPTY) break;

        if ((uint8_t) entry->name[0] == DELETED) continue;

        if (strcmp(path, ".") == 1) continue;

        void *dir = kalloc(11);
        fat32_get_name(entry, dir);

        if (strcmp(dir, path) != 1) {
            kfree(dir);
            continue;
        }

        // Le chemin contient un ficher et pas un dossier
        if (entry->attributes != DIRECTORY) {
            status = 1;
            kfree(dir);
            break;
        }


        if (strcmp(path, "..") == 1) {
            go_upper_directory();
            kfree(dir);
            continue;
        }

        const uint32_t target_cluster = (entry->cluster_entry_high << 16) | entry->cluster_entry_low;

        add_path(path);

        set_current_directory(target_cluster);

        kfree(dir);
    }

    kfree(buffer);

    return status;
}

int fat32_change_dir(const char *directory_name) {
    int status = 0;
    char p[128];
    char *p_ptr = p;
    strcpy(p, directory_name);

    if (directory_name[0] == '/') {
        go_to_root_dir();
    }

    char *split_path[16];
    const int parts = strsplit(p_ptr, split_path, 16, '/');

    for (int i = 0; i < parts; i++) {
        if (status == 1) break;
        status = change_directory_depth(split_path[i]);
    }

    return status;
}

int fat32_create_directory(const char *directory) {
    const uint32_t global_cluster = fat32_allocate_cluster();
    const uint32_t lba_sector = get_starting_data_sector() + (global_cluster - 2) * get_sectors_per_cluster();

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

    if (get_current_directory() == 2) {
        parent_dir->cluster_entry_high = 0;
        parent_dir->cluster_entry_low = 0;
    } else {
        parent_dir->cluster_entry_high = get_current_directory() >> 16;
        parent_dir->cluster_entry_low = get_current_directory() & 0xFFFF;
    }

    sd_write_block(lba_sector, dir_content);

    kfree(dir_content);

    void *parent_directory = kalloc(512);
    uint32_t parent_sector = get_starting_data_sector() + (get_current_directory() - 2) * get_sectors_per_cluster();
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

    return 0;
}



int fat32_remove_directory(const char *path) {
    void *directory_buffer = kalloc(512);
    sd_read_block(get_current_directory_sector(), directory_buffer);


    for (int i = 0; i < 16; i++) {
        fat32_dir_entry *entry = &directory_buffer[i * 32];

        if ((uint8_t) entry->name[0] == EMPTY) {
            kfree(directory_buffer);
            return 0;
        }

        if ((uint8_t) entry->name[0] == DELETED) continue;


        void *file_name = kalloc(11);
        fat32_get_name(entry, file_name);

        if (strcmp(file_name, path) == 0) {
            kfree(file_name);
            continue;
        }

        if (entry->attributes != DIRECTORY) {
            kfree(file_name);
            return 1;
        }

        const uint32_t cluster = (entry->cluster_entry_high << 16) | entry->cluster_entry_low;

        void *file_buffer = kalloc(512);
        const uint32_t lba_sector = get_starting_data_sector() + (cluster - 2) * get_sectors_per_cluster();
        sd_read_block(lba_sector, file_buffer);
        const fat32_dir_entry *entries = file_buffer;

        fat32_change_dir(path);
        for (int j = 0; j < 16; j++) {
            if (entries[j].name[0] == EMPTY) break;

            if (entries[j].name[0] == DELETED) continue;

            if (entries[j].name[0] == '.' && (entries[j].name[1] == ' ' || entries[j].name[1] == '.')) continue;


            if (entries[j].attributes == DIRECTORY) {
                fat32_remove_directory(entries[j].name);
            } else {
                char out[11];
                fat32_get_name(&entries[j], out);
                fat32_remove_file(out);
            }
        }
        fat32_change_dir("..");

        fat32_delete_entry_raw(entry, directory_buffer);
        kfree(file_buffer);
        kfree(file_name);

        return 0;
    }


    kfree(directory_buffer);
    return 1;
}


// TODO: Gérer les LONG_FILE_NAME
int fat32_list_dir(const char *path, char ***out_file_list) {
    char orig_path[128];
    strcpy(orig_path, current_path);

    if (strcmp(path, "") == 0)
        fat32_change_dir(path);

    void *buffer = kalloc(512);
    char **temp_array = kalloc(128 * sizeof(char *));
    sd_read_block(get_current_directory_sector(), buffer);

    int file_number = 0;

    for (int i = 0; i < 16; i++) {
        const fat32_dir_entry *entry = &buffer[i * 32];
        if ((uint8_t) entry->name[0] == EMPTY) break;

        if ((uint8_t) entry->name[0] == DELETED) continue;

        if (entry->attributes == LONG_FILE_NAME) {
            // Long names
        }

        void *file_name = kalloc(strlen(entry->name) + 1);
        fat32_get_name(entry, file_name);
        temp_array[file_number] = file_name;
        file_number++;
    }

    *out_file_list = temp_array;
    kfree(buffer);

    fat32_change_dir(orig_path);
    return file_number;
}