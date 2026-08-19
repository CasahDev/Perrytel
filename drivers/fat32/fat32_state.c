//
// Created by sacha on 19/08/2026.
//

#include <stdint.h>

static uint32_t starting_partition_sector;
static uint32_t starting_fat_sector;
static uint32_t starting_data_sector;
static uint32_t sectors_per_cluster;
static uint32_t root_sector;
static uint32_t current_directory_cluster = 2;

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

uint32_t set_starting_partition_sector(const uint32_t start) {
    starting_partition_sector = start;
}

uint32_t set_starting_fat_sector(const uint32_t start) {
    starting_fat_sector = start;
}

uint32_t set_starting_data_sector(const uint32_t start) {
    starting_data_sector = start;
}

uint32_t set_sectors_per_cluster(const uint32_t start) {
    sectors_per_cluster = start;
}

uint32_t set_root_sector(const uint32_t start) {
    root_sector = start;
}