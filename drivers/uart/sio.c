//
// Created by sacha on 10/29/26.
//

#include <kernel/sio-uart.h>

#include <drivers/uart.h>

sio_fd_t sio_open(uint8_t devnum) {
    return (void *) 1;
}

void sio_send(const uint8_t c, sio_fd_t dev) {
    (void)dev;
    uart_putc(c);
}

uint32_t sio_tryread(sio_fd_t dev, u8_t *data, u32_t len) {
    return 0;
}
