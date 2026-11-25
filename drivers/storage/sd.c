//
// Created by sacha on 30/07/2026.
//
#include <stdint.h>
#include <drivers/sd.h>
#include <drivers/fat32.h>

#include <kernel/mmio.h>
#include <kernel/memory.h>

#define CONTROL1_POS 0x2C
#define ARG1 0x08
#define CMDTM 0x0C
#define INTERRUPT 0x30
#define RESP0 0x10
#define STATUS 0x24
#define DATA 0x20

#define RESPONSE 0x1A << 16
#define RESPONSE_R3 0x02 << 16
#define NO_RESPONSE 0x00 << 16

#define CMD0 (0 << 24 | NO_RESPONSE)
// Récupère le numéro de série et lui fait générer son RCA
#define CMD2 (2 << 24 | RESPONSE)

// Récupère l'adresse RCA de la carte SD
#define CMD3 (3 << 24 | RESPONSE)

// Selection de la carte via l'argument (adresse RCA)
#define CMD7 (7 << 24 | RESPONSE)

//Check la tension de la carte
#define CMD8 (8 << 24 | RESPONSE)

// Définie la taille des blocks de mémoire que l'on gère
#define CMD16 (16 << 24 | RESPONSE)

// Lire un block de donnée
#define CMD17 (17 << 24 | 1 << 21 | 1 << 4 | RESPONSE)

// Demande a écrire dans un block de donnée
#define CMD24 (24 << 24 | 1 << 21 | 0 << 4 | RESPONSE)

// Commande spéciale pour négocier le support de la haute capacité
#define ACMD41 (41 << 24 | RESPONSE_R3)

// Préviens la carte sd de l'utilisation d'une commande spéciale
#define CMD55 (55 << 24 | RESPONSE)

#define NO_ARGS 0x0
#define HCS_OCR (0x1 << 30 | 0xFF8000)
#define TENSION_TEST 0x1AA
#define BLOCK_SIZE 512
#define RESET_SD_CARD 0x00 << 24
#define BLKSIZECNT 0x04

#define GLOBAL_RESET 1 << 24
#define INTERNAL_CLOCK_ENABLE 0x1
#define INTERNAL_CLOCK_STABLE 0x1 << 1
#define SD_CLOCK_ENABLE 0x1 << 2
#define CLOCK_DIVISOR 0x7C << 8
#define HIGH_CLOCK_DIVISOR 2 << 8

#define BOOT_FLAG 0x1BE

#define FAT32_LBA 0x0C
#define FAT32_CHS 0x0B

#define EMMC_IRPT_MASK 0x34
#define EMMC_IRPT_EN 0x38
#define EMMC_IRPT_ALL 0xFFFFFFFF

#define SD_OCR_POWER_UP_DONE (1 << 31)
#define SD_OCR_CARD_CAPACITY (1 << 30)

#define SD_RCA_MASK 0xFFFF0000

#define EMMC_STATUS_BUF_READ_ENABLE (1 << 11)
#define EMMC_STATUS_BUF_WRITE_ENABLE (1 << 10)

#define SD_WORDS_PER_BLOCK (BLOCK_SIZE / sizeof(uint32_t))

#define EMMC_INT_DATA_DONE (1 << 1)
#define EMMC_INT_CMD_DONE (1 << 0)

#define MBR_PARTITION_TYPE_OFFSET 4
#define MBR_PARTITION_LBA_OFFSET 8
#define MBR_PARTITION_SIZE_OFFSET 12

uint8_t is_sdhc;

const struct vfs_driver *active_fs = NULL;

static void send_command(const uint32_t cmdtm, const uint32_t args) {
    mmio_write(EMMC, ARG1, args);
    mmio_write(EMMC, CMDTM, cmdtm);
    while ((mmio_read(EMMC, INTERRUPT) & EMMC_INT_CMD_DONE) == 0x0) {
    }
    mmio_write(EMMC, INTERRUPT, 0xFFFFFFFF);
}

void sd_init() {
    uint32_t control1 = mmio_read(EMMC, CONTROL1_POS);
    control1 |= GLOBAL_RESET;
    mmio_write(EMMC, CONTROL1_POS, control1);
    while ((mmio_read(EMMC, CONTROL1_POS) >> 24 & 0x1) == 1) {
    }

    // Négotiation horloge controlleur
    control1 = mmio_read(EMMC, CONTROL1_POS);
    control1 |= INTERNAL_CLOCK_ENABLE;
    control1 &= ~(0xFF << 8);
    control1 |= CLOCK_DIVISOR;
    mmio_write(EMMC, CONTROL1_POS, control1);
    while ((mmio_read(EMMC, CONTROL1_POS) & INTERNAL_CLOCK_STABLE) == 0) {
    }

    control1 = mmio_read(EMMC, CONTROL1_POS);
    control1 |= SD_CLOCK_ENABLE;
    mmio_write(EMMC, CONTROL1_POS, control1);

    // Besoin d'attendre un peu pour initialiser la carte SD
    for (int i = 0; i < 10000; i++) {
    }

    // Réinitialisation connexion carte SD
    mmio_write(EMMC, ARG1, NO_ARGS);
    mmio_write(EMMC, CMDTM, 0x0 << 24);

    // Masques d'interruption
    mmio_write(EMMC, EMMC_IRPT_MASK, EMMC_IRPT_ALL);
    mmio_write(EMMC, EMMC_IRPT_EN, EMMC_IRPT_ALL);

    // SEND_IF_COND
    send_command(CMD8, TENSION_TEST);

    send_command(CMD0, NO_ARGS);

    while ((mmio_read(EMMC, RESP0) & SD_OCR_POWER_UP_DONE) == 0x0) {
        send_command(CMD55, NO_ARGS);

        send_command(ACMD41, HCS_OCR);
        for (int i = 0; i < 100000; i++) {
        }
    }

    is_sdhc = (mmio_read(EMMC, RESP0) & SD_OCR_CARD_CAPACITY) != 0;

    send_command(CMD2, NO_ARGS);

    send_command(CMD3, NO_ARGS);

    const uint32_t rca = mmio_read(EMMC, RESP0) & SD_RCA_MASK;

    send_command(CMD7, rca);

    send_command(CMD16, BLOCK_SIZE);

    control1 = mmio_read(EMMC, CONTROL1_POS);
    control1 &= ~(0xFF << 8);
    control1 |= HIGH_CLOCK_DIVISOR;
    mmio_write(EMMC, CONTROL1_POS, control1);
    while ((mmio_read(EMMC, CONTROL1_POS) & INTERNAL_CLOCK_STABLE) == 0) {}

    uint8_t *buffer = kalloc(BLOCK_SIZE);
    sd_read_block(0, buffer);

    const uint8_t file_system = buffer[BOOT_FLAG + MBR_PARTITION_TYPE_OFFSET];
    const uint32_t starting_sector = *(uint32_t *) &buffer[BOOT_FLAG + MBR_PARTITION_LBA_OFFSET];
    // const uint32_t partition_size = *(uint32_t *) &buffer[BOOT_FLAG + MBR_PARTITION_SIZE_OFFSET];

    sd_read_block(starting_sector, buffer);

    switch (file_system) {
        case FAT32_CHS:
        case FAT32_LBA:
            fat32_init(starting_sector, buffer);
            break;
    }

    active_fs = &fat32_driver;

    kfree(buffer);
}

void sd_read_block(const uint32_t sector, uint8_t *buffer) {
    mmio_write(EMMC, BLKSIZECNT, (1 << 16) | BLOCK_SIZE);
    send_command(CMD17, is_sdhc ? sector : sector * BLOCK_SIZE);

    while ((mmio_read(EMMC, STATUS) & EMMC_STATUS_BUF_READ_ENABLE) == 0) {
    }

    for (uint32_t i = 0; i < SD_WORDS_PER_BLOCK; i++) {
        const uint32_t data = mmio_read(EMMC, DATA);
        ((uint32_t *) buffer)[i] = data;
    }

    while ((mmio_read(EMMC, INTERRUPT) & EMMC_INT_DATA_DONE) == 0) {
    }
    mmio_write(EMMC, INTERRUPT, EMMC_INT_DATA_DONE);
}

void sd_write_block(uint32_t sector, const uint8_t *buffer) {
    mmio_write(EMMC, BLKSIZECNT, 1 << 16 | BLOCK_SIZE);

    send_command(CMD24, is_sdhc ? sector : sector * BLOCK_SIZE);

    while ((mmio_read(EMMC, STATUS) & EMMC_STATUS_BUF_WRITE_ENABLE) == 0) {
    }

    for (uint32_t i = 0; i < SD_WORDS_PER_BLOCK; i++) {
        mmio_write(EMMC, DATA, ((uint32_t *)buffer)[i]);
    }

    while ((mmio_read(EMMC, INTERRUPT) & EMMC_INT_DATA_DONE) == 0) {
    }
    mmio_write(EMMC, INTERRUPT, EMMC_INT_DATA_DONE);
}