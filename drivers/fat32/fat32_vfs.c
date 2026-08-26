//
// Created by sacha on 19/08/2026.
//
#include <kernel/vfs.h>

#include "fat32_internal.h"

const struct vfs_driver fat32_driver = {
    .name = "FAT32",
    .change_dir = fat32_change_dir,
    .list_dir = fat32_list_dir,
    .path = get_path,
    .read_file = fat32_read_file,
    .create_dir = fat32_create_directory,
    .create_file = fat32_create_file,
    .remove_file = fat32_remove_file,
    .remove_dir = fat32_remove_directory
};
