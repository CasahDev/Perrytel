//
// Created by sacha on 10/29/26.
//

#pragma once

#include "lwip/sio.h"

void sio_handler();

sio_fd_t sio_open(uint8_t devnum);

void sio_send(uint8_t c, sio_fd_t dev);

uint32_t sio_tryread(sio_fd_t dev, u8_t *data, u32_t len);