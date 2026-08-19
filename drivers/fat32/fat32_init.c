#include <drivers/fat32.h>
#include <drivers/sd.h>

#include "fat32_internal.h"


void fat32_init(const uint32_t starting_sector, const uint8_t *cluster) {
    const fat32_boot_sector *sector = (const fat32_boot_sector *) cluster;

    set_starting_partition_sector(starting_sector);
    set_starting_fat_sector(get_starting_partition_sector() + sector->reserved_sector_number);
    set_starting_data_sector(get_starting_fat_sector() + sector->sectors_per_fat * sector->fats_number);
    set_sectors_per_cluster(sector->sectors_per_cluster);
    set_root_sector(get_starting_data_sector() + (sector->root_directory_cluster - 2) * get_sectors_per_cluster());
}