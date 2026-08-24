#pragma once

#include <drivers/fat32.h>

// ##############################################################################################
// ####################################### ACCESSORS ############################################
// ##############################################################################################

uint32_t get_starting_partition_sector();

uint32_t get_starting_fat_sector();

uint32_t get_starting_data_sector();

uint32_t get_sectors_per_cluster();

uint32_t get_root_sector();

uint32_t get_current_directory();

void set_current_directory(const uint32_t new_directory);

uint32_t get_current_directory_sector();

uint32_t set_starting_partition_sector(const uint32_t start);

uint32_t set_starting_fat_sector(const uint32_t start);

uint32_t set_starting_data_sector(const uint32_t start);

uint32_t set_sectors_per_cluster(const uint32_t start);

uint32_t set_root_sector(const uint32_t start);

// ##############################################################################################
// ######################################### FILES ##############################################
// ##############################################################################################

void fat32_get_name(const fat32_dir_entry *entry, char *out_name);

int fat32_delete_entry_raw(fat32_dir_entry *entry, void *directory_buffer);

int fat32_read_file(const char *path, char *content, uint32_t max_size);

int fat32_remove_file(const char *path);

int fat32_create_file(const char *file_name, const char *content);

// ##############################################################################################
// ######################################### ALLOC ##############################################
// ##############################################################################################

uint32_t fat32_allocate_cluster();

// ##############################################################################################
// ##################################### DIRECTORIES ############################################
// ##############################################################################################
int fat32_change_dir(const char *directory_name);

int fat32_create_directory(const char *directory);

int fat32_remove_directory(const char *path);

int fat32_list_dir(const char *path, char ***out_file_list);

int get_path(char *out_path);