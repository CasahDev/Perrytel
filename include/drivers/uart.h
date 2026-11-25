#ifndef UART_H
#define UART_H

#include <stdint.h>

#include <kernel/vfs.h>

enum {
    // The offsets for reach register.
    GPIO_BASE = 0x200000,
    GPFSEL1 = (GPIO_BASE + 0x04),
    GPPUD = (GPIO_BASE + 0x94),
    GPPUDCLK0 = (GPIO_BASE + 0x98),
    UART0_BASE = (GPIO_BASE + 0x1000),
    UART0_DR = (UART0_BASE + 0x00),
    UART0_RSRECR = (UART0_BASE + 0x04),
    UART0_FR = (UART0_BASE + 0x18),
    UART0_ILPR = (UART0_BASE + 0x20),
    UART0_IBRD = (UART0_BASE + 0x24),
    UART0_FBRD = (UART0_BASE + 0x28),
    UART0_LCRH = (UART0_BASE + 0x2C),
    UART0_CR = (UART0_BASE + 0x30),
    UART0_IMSC = (UART0_BASE + 0x38),
    UART0_MIS = (UART0_BASE + 0x40),
    UART0_ICR = (UART0_BASE + 0x44),
    MBOX_BASE = 0xB880,
    MBOX_STATUS = (MBOX_BASE + 0x18),
    MBOX_WRITE = (MBOX_BASE + 0x20),
    MBOX_READ = (MBOX_BASE + 0x00),
    UART1_BASE = 0x215000,
    AUX_ENABLES = (UART1_BASE + 0x04),
    UART1_IO_REG = (UART1_BASE + 0x40),
    UART1_IER_REG = (UART1_BASE + 0x44),
    UART1_LSR_REG = (UART1_BASE + 0x54),


    // Mailbox command ID to fix clock rate
    RPI_MBOX_TAG_SET_CLOCK_RATE = 0x38002,

    // Clock rate for 3MHZ for GPU
    UART_CLOCK_RATE_3MHZ = 3000000,

    // Base frequency for divisor
    UART_REF_BLOCK_12MHZ = 12000000,

    // Enable uart
    UART_CR_UARTEN = 1 << 0,

    // Enable uart sender
    UART_CR_TXE = 1 << 8,

    // enable uart receiver
    UART_CR_RXE = 1 << 9,

    // Length of UART word
    UART_LCRH_PEN = 1 << 1,

    // Chose parity type
    UART_LCRH_EPS = 1 << 2,

    // Enable FIFO
    UART_LCRH_FEN = 1 << 4,

    // Chose lcrh word length
    UART_LCRH_WLEN_8BIT =  1 << 6 | 1 << 5,

    // Empty FIFO output flag
    UART_FR_RXFE = 1 << 4,

    // Empty mini uart FIFO input flag
    UART1_LSR_TX_EMPTY = 1 << 5,

    // TARGET GPIO 14 and 15
    GPIO_PIN14_FSEL_SHIFT = 12,
    GPIO_PIN14_FSEL_VALUE = 7,
    GPIO_PIN15_FSEL_SHIFT = 15,
    GPIO_PIN15_FSEL_VALUE = 7,

    // Remove previous function on PIN 14
    GPIO_SFEL_MASK = 0x7,

    // ALT 0 function in FPIO
    GPIO_SFEL_ALT0 = 4,

    // Forced number of cycles for pull-up/down signals to stabilize itself
    GPIO_PUD_SETTLE_CYCLES = 150,

    // Control clock signal for pin 14
    GPIO_PIN14_BIT = 1 << 14,
    // Control clock signal for pin 15
    GPIO_PIN15_BIT = 1 << 15,

    // Acquit all interruptins at start
    UART_ICR_CLEAR_ALL = 0x7FF,

    // Reception interruption -> Data available in fifo
    UART_INT_RX = 1 << 4,
    // Send interruption -> TX buffer empty and ready for new data
    UART_INT_TX = 1 << 5,
    // Data waiting in fifo
    UART_INT_RT = 1 << 6,
    // Frame error interruption
    UART_INT_FE = 1 << 7,
    // Parity error interruption
    UART_INT_PE = 1 << 8,
    // Break error interruption
    UART_INT_BE = 1 << 9,
    // Overrun error interruption
    UART_INT_OE = 1 << 10,
    UART_INT_RIM = (1 << 1),

    // Property tag
    MBOX_CHANNEL_PROPERTY = 8,

    // Alignment mast for GPU
    MBOX_ALIGNMENT_MASK = ~0xF,

    // Mailbox waiting line is full
    MBOX_STATUS_FULL = 0x80000000,

    // Mailbox is empty
    MBOX_STATUS_EMPTY = 0x40000000,

    // MBOX id
    RPI_MBOX_CLOCK_ID_UART = 2,

    // Mbox buffer size,
    MBOX_BUFFER_SIZE = 12,

    // Enable mini UART
    AUX_ENABLES_MINI_UART = 1,
    UART1_IER_RX_INT = 1,
    UART1_LSR_DATA_READY = 0x1,

    UART_BUFFER_SIZE = 1024,
    SHELL_BUFFER_SIZE = 256,
    ASCII_DEL = 0x7F,

    UART_BAUD_RATE_1200 = 1200
};

extern const struct vfs_driver uart_vfs_driver;

void uart_init(int raspi);

void uart_putc(unsigned char c);

void uart_puts(const char *str);

void uart_put_hex(uint64_t hex);

void uart_handle_interrupt();

int uart_get_char_async();

void execute_command(void);

void printf(char *str);

bool is_command_ready();

#endif
