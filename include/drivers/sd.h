#pragma once

#define EMMC 0xFE300000

void sd_init();

void sd_read_block(uint32_t sector, uint8_t *buffer);