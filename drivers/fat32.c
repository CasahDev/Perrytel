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

static char current_path[128] = "/";

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


static void fat32_get_name(const fat32_dir_entry *entry, char *out_name) {
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

// TODO Gérer les fichiers dans des sous dossiers
static int fat32_read_file(const char *path, char *content, uint32_t max_size) {
    void *buffer = kalloc(512);

    sd_read_block(get_current_directory_sector(), buffer);

    for (int i = 0; i < 16; i++) {
        const fat32_dir_entry *entry = &buffer[i * 32];
        if ((uint8_t) entry->name[0] != EMPTY && (uint8_t) entry->name[0] != DELETED) {
            void *file_name = kalloc(11);
            fat32_get_name(entry, file_name);
            if (strcmp(file_name, path) != 0) {
                uint32_t cluster_num = (entry->cluster_entry_high << 16) + entry->cluster_entry_low;
                uint32_t bytes_read = 0;
                void *fat_buffer = kalloc(512);

                while (cluster_num < EOF && bytes_read < entry->size) {
                    if (bytes_read + 512 > max_size) {
                        kfree(fat_buffer);
                        kfree(file_name);
                        kfree(buffer);
                        return 2;
                    }

                    const uint32_t lba_sector = starting_data_sector + (cluster_num - 2) * sectors_per_cluster;

                    sd_read_block(lba_sector, (uint8_t *) content + bytes_read);
                    bytes_read += 512;

                    const uint32_t fat_sector = starting_fat_sector + cluster_num / 128;
                    sd_read_block(fat_sector, fat_buffer);
                    cluster_num = ((uint32_t *) fat_buffer)[cluster_num % 128];
                }

                kfree(fat_buffer);
                content[entry->size] = '\0';
                kfree(file_name);
                kfree(buffer);
                return 0;
            }
            kfree(file_name);
        }
    }


    return 1;
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

static int fat32_create_file(const char *file_name, const char *content) {
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

    sd_write_block(get_current_directory_sector(), root_buffer);

    kfree(root_buffer);

    kfree(file_content);

    return 0;
}

// TODO: gérer les changements de dossier à plusieurs étages -> ex: cd home/xxx/Documents
static int fat32_change_dir(const char *directory_name) {
    void *buffer = kalloc(512);
    sd_read_block(get_current_directory_sector(), buffer);
    uint8_t status = 0;

    const char *directory = directory_name;

    for (int i = 0; i < 16; i++) {
        const fat32_dir_entry *entry = &buffer[i * 32];

        if ((uint8_t) entry->name[0] == EMPTY) break;

        if ((uint8_t) entry->name[0] == DELETED) continue;

        void *dir = kalloc(11);
        fat32_get_name(entry, dir);

        if (strcmp(dir, directory) != 1) continue;

        if (entry->attributes != DIRECTORY) status = 1;

        uint32_t target_cluster = (entry->cluster_entry_high << 16) | entry->cluster_entry_low;
        if (target_cluster == 0) {
            target_cluster = 2;
            memset(current_path, ' ', 128);
            current_path[0] = '/';
            current_path[1] = '\0';
        }

        if (strcmp(directory, "..") == 1) {
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
        } else {
            if (current_path[strlen(current_path) - 1] != '/') {
                current_path[strlen(current_path)] = '/';
                current_path[strlen(current_path)] = '\0';
            }

            uint32_t current_pos = strlen(current_path);
            for (uint16_t l = 0; l < strlen(directory); l++) {
                current_path[current_pos] = directory[l];
                current_pos++;
            }
            current_path[current_pos] = '\0';
        }

        set_current_directory(target_cluster);

        kfree(dir);
    }

    kfree(buffer);

    return status;
}

static int fat32_create_directory(const char *directory) {
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

    return 0;
}

static int fat32_delete_entry_raw(fat32_dir_entry *entry, void *directory_buffer) {
    uint32_t sector = get_current_directory_sector();

    entry->name[0] = DELETED;

    sd_write_block(sector, directory_buffer);

    uint32_t cluster = (entry->cluster_entry_high << 16) | entry->cluster_entry_low;

    void *file_buffer = kalloc(512);
    while (cluster < EOF) {
        const uint32_t lba_sector = starting_fat_sector + cluster / 128;
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

static int fat32_remove_file(const char *path) {
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

        if (entry->attributes == DIRECTORY) {
            return 1;
        }

        fat32_delete_entry_raw(entry, buffer);

        return 0;
    }

    kfree(buffer);
    return 1;
}


static int fat32_remove_directory(const char *path) {
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
        const uint32_t lba_sector = starting_data_sector + (cluster - 2) * sectors_per_cluster;
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


// TODO: Gérer les LONG_FILE_NAME & le passage du path
int fat32_list_dir(const char *path, char ***out_file_list) {
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
    return file_number;
}

static int path(char *out_path) {
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

// endregion


const struct vfs_driver fat32_driver = {
    .name = "FAT32",
    .change_dir = fat32_change_dir,
    .list_dir = fat32_list_dir,
    .path = path,
    .read_file = fat32_read_file,
    .create_dir = fat32_create_directory,
    .create_file = fat32_create_file,
    .remove_file = fat32_remove_file,
    .remove_dir = fat32_remove_directory
};
