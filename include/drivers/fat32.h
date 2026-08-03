#pragma once
#include <stdint.h>

typedef struct __attribute__((packed)) fat32_boot_sector {
    // BPB
    uint8_t reserved[3];
    char oem_name[8];

    // En little endian
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_number;

    // Souvent 2
    uint8_t fats_number;
    uint16_t root_directory_number;

    // Si > 65535 alors stocké dans le large_sector_count
    uint16_t sector_count;
    uint8_t media_descriptor_type;

    // Toujours à 0 en FAT32
    uint16_t reserved_sectors_per_fat;
    uint16_t sector_per_track;
    uint16_t heads_per_storage_media;
    uint32_t hidden_sector_count;
    uint32_t large_sector_count;

    // EBR
    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t fat_version;

    // Souvent 2
    uint32_t root_directory_cluster;
    uint16_t fsinfo_sector;
    uint16_t backup_boot_sector;

    // Octets a 0 si le volume est formaté
    uint8_t reserved_2[12];
    uint8_t drive_number;

    // Flags pour Windows NT
    uint8_t reserved_3;

    // Soit 0x28 ou 0x29
    uint8_t signature;
    uint32_t volume_id;
    char volume_label_string[11];

    // Toujours "FAT32   "
    char system_id_string[8];
    uint8_t boot_code[420];

    // 0xAA55
    uint16_t bootable_partition_signature;
} fat32_boot_sector;

typedef struct __attribute__((packed)) FSInfo {
    // Vaut 0x41615252 pour une structure FS info valide
    uint32_t lead_signature;

    uint8_t reserved[480];

    // Vaut 0x61417272
    uint32_t another_signature;

    // Doit toujours être check par rapport au nombre de cluster + si = 0xFFFFFFFF, nb inconnu
    uint32_t last_known_free_cluster;

    /** numéro du cluster le plus récemment alloué, qui sert au système de fichiers à savoir où chercher le prochain
     * cluster libre sans relire toute la table FAT (si vaut 0xFFFFFFFF, nb inconnu)
    */
    uint32_t next_free;
    uint8_t reserved_2[12];

    // Vaut 0xAA550000
    uint32_t trail_signature;
} FSInfo;


void fat32_init(uint32_t starting_sector, uint8_t *cluster);