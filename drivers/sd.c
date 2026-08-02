//
// Created by sacha on 30/07/2026.
//
#include <stdint.h>
#include <drivers/sd.h>

#include <kernel/mmio.h>

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

// Commande spéciale pour négocier le support de la haute capacité
#define ACMD41 (41 << 24 | RESPONSE_R3)

// Préviens la carte sd de l'utilisation d'une commande spéciale
#define CMD55 (55 << 24 | RESPONSE)

#define NO_ARGS 0x0
#define HCS_OCR (0x1 << 30 | 0xFF8000)
#define TENSION_TEST 0x1AA
#define BLOCK_SIZE 512
#define RESET_SD_CARD 0x00 << 24

#define GLOBAL_RESET 1 << 24
#define INTERNAL_CLOCK_ENABLE 0x1
#define INTERNAL_CLOCK_STABLE 0x1 << 1
#define SD_CLOCK_ENABLE 0x1 << 2
#define CLOCK_DIVISOR 0x7C << 8
#define HIGH_CLOCK_DIVISOR 2 << 8

static void send_command(const uint32_t cmdtm, const uint32_t args) {
    mmio_write(EMMC, ARG1, args);
    mmio_write(EMMC, CMDTM, cmdtm);
    while ((mmio_read(EMMC, INTERRUPT) & 0x1) == 0x0) {
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
    while ((mmio_read(EMMC, CONTROL1_POS) >> 1 & 0x1) == 0) {
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
    mmio_write(EMMC, 0x34, 0xFFFFFFFF);
    mmio_write(EMMC, 0x38, 0xFFFFFFFF);

    // SEND_IF_COND
    send_command(CMD8, TENSION_TEST);

    send_command(0, NO_ARGS);

    while ((mmio_read(EMMC, RESP0) >> 31 & 0x1) == 0x0) {
        send_command(CMD55, NO_ARGS);

        send_command(ACMD41, HCS_OCR);
        for (int i = 0; i < 100000; i++) {
        }
    }

    send_command(CMD2, NO_ARGS);

    send_command(CMD3, NO_ARGS);

    const uint32_t rca = mmio_read(EMMC, RESP0) & 0xFFFF0000;

    send_command(CMD7, rca);

    send_command(CMD16, BLOCK_SIZE);

    control1 = mmio_read(EMMC, CONTROL1_POS);
    control1 &= ~(0xFF << 8);
    control1 |= HIGH_CLOCK_DIVISOR;
    mmio_write(EMMC, CONTROL1_POS, control1);
    while ((mmio_read(EMMC, CONTROL1_POS) >> 1 & 0x1) == 0) {}

}

void sd_read_block(const uint32_t sector, uint8_t *buffer) {
    mmio_write(EMMC, 0x04, (1 << 16) | 512);
    send_command(CMD17, sector);

    while ((mmio_read(EMMC, STATUS) >> 11 & 0x1) == 0) {
    }

    for (int i = 0; i < 128; i++) {
        const uint32_t data = mmio_read(EMMC, DATA);
        ((uint32_t *) buffer)[i] = data;
    }

    while ((mmio_read(EMMC, INTERRUPT) >> 1 & 0x1) == 0) {
    }
    mmio_write(EMMC, INTERRUPT, 0x2);
}
