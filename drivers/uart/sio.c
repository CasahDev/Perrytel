//
// Created by sacha on 10/29/26.
//

#include <kernel/sio-uart.h>

#include <drivers/uart.h>

#include <kernel/mmio.h>

#define BUFFER_LEN 4096

volatile unsigned char buffer[BUFFER_LEN];
volatile uint32_t head;
volatile uint32_t tail;


/**
 * Interrupt handler used to save data received from the auxiliary
 * uart into the buffer variable
 */
void sio_handler() {
    while (mmio_read(MMIO_BASE, UART1_LSR_REG) & 0x1) {
        buffer[head] = mmio_read(MMIO_BASE, UART1_IO_REG);
        head = (head + 1) % BUFFER_LEN;
    }
}


sio_fd_t sio_open(uint8_t devnum) {
    return (void *) 1;
}

/**
 * Send an unsigned char to the slip interface
 * @param c The char to send
 * @param dev The interface to use
 */
void sio_send(const uint8_t c, sio_fd_t dev) {
    (void)dev;
    while (!(mmio_read(MMIO_BASE, UART1_LSR_REG) >> 5 & 0x1)) {}
    mmio_write(MMIO_BASE, UART1_IO_REG, c);
}

/**
 * Return the number for bytes read on the SLIP and write it on the data variable
 * @param dev The slip descriptor
 * @param data The destination of the received data
 * @param len The length of the table
 * @return The number of bytes read
 */
uint32_t sio_tryread(sio_fd_t dev, u8_t *data, const u32_t len) {
    uint32_t i = 0;
    (void) dev;
    while (i < len && tail != head) {
        data[i] = buffer[tail];
        i++;
        tail = (tail + 1) % BUFFER_LEN;
    }

    return i;
}
