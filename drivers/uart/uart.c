#include <stdbool.h>
#include <stdint.h>

#include <drivers/uart.h>

#include <kernel/memory.h>
#include <kernel/mmio.h>
#include <kernel/shell.h>

#include <string.h>


uint32_t MMIO_BASE;

volatile unsigned char tx_buffer[UART_BUFFER_SIZE];
volatile uint32_t tx_head;
volatile uint32_t tx_tail;

volatile unsigned char rx_buffer[UART_BUFFER_SIZE];
volatile uint32_t rx_head;
volatile uint32_t rx_tail;

char shell_buffer[256];
volatile int shell_index;
volatile bool command_ready = false;

bool is_command_ready() {
    return command_ready;
}

// Loop <delay> times in a way that the compiler won't optimize away
static inline void delay(int32_t count) {
    __asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
        : "=r"(count): [count]"0"(count) : "cc");
}

volatile unsigned int __attribute__((aligned(16))) mbox[9] = {
    9 * 4, 0, RPI_MBOX_TAG_SET_CLOCK_RATE, MBOX_BUFFER_SIZE, MBOX_CHANNEL_PROPERTY, RPI_MBOX_CLOCK_ID_UART, UART_CLOCK_RATE_3MHZ, 0, 0
};

void uart_set_baudrate(uint32_t baud) {
    mmio_write(MMIO_BASE, UART0_CR, 0);
    uint32_t divisor = UART_REF_BLOCK_12MHZ / baud;
    uint32_t ibrd = divisor / 64;
    uint32_t fbrd = divisor % 64;
    mmio_write(MMIO_BASE, UART0_IBRD, ibrd);
    mmio_write(MMIO_BASE, UART0_FBRD, fbrd);
    mmio_write(MMIO_BASE, UART0_LCRH, UART_LCRH_PEN | UART_LCRH_EPS | UART_LCRH_FEN | UART_LCRH_WLEN_8BIT);
    mmio_write(MMIO_BASE, UART0_CR, UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE);
}

const struct vfs_driver uart_vfs_driver = {
    .file_exists = NULL,
    .open = NULL,
    .read_file = NULL,
    .change_dir = NULL,
    .create_dir = NULL,
    .create_file = NULL,
    .list_dir = NULL,
    .name = NULL,
    .path = NULL,
    .remove_dir = NULL,
    .remove_file = NULL,
};

void uart_init(int raspi) {
    switch (raspi) {
        case 2:
        case 3: MMIO_BASE = 0x3F000000;
            break;
        case 4: MMIO_BASE = 0xFE000000;
            break;
        default: MMIO_BASE = 0x20000000;
            break;
    }

    mmio_write(MMIO_BASE, UART0_CR, 0x00000000);

    uint32_t selector = mmio_read(MMIO_BASE, GPFSEL1);
    selector &= ~((GPIO_PIN14_FSEL_VALUE << GPIO_PIN14_FSEL_SHIFT) | (GPIO_PIN15_FSEL_VALUE << GPIO_PIN15_FSEL_SHIFT));
    selector |= (GPIO_SFEL_ALT0 << GPIO_PIN14_FSEL_SHIFT) | (GPIO_SFEL_ALT0 << GPIO_PIN15_FSEL_SHIFT);
    mmio_write(MMIO_BASE, GPFSEL1, selector);

    mmio_write(MMIO_BASE, GPPUD, 0x00000000);
    delay(GPIO_PUD_SETTLE_CYCLES);
    mmio_write(MMIO_BASE, GPPUDCLK0, GPIO_PIN14_BIT | GPIO_PIN15_BIT);
    delay(GPIO_PUD_SETTLE_CYCLES);
    mmio_write(MMIO_BASE, GPPUDCLK0, 0x00000000);
    mmio_write(MMIO_BASE, UART0_ICR, UART_ICR_CLEAR_ALL);

#ifndef QEMU
    if (raspi >= 3) {
        unsigned int r = (((uintptr_t) (&mbox) & MBOX_ALIGNMENT_MASK) | MBOX_CHANNEL_PROPERTY);
        while (mmio_read(MMIO_BASE, MBOX_STATUS) & MBOX_STATUS_FULL) {
        }
        mmio_write(MMIO_BASE, MBOX_WRITE, r);
        while ((mmio_read(MMIO_BASE, MBOX_STATUS) & MBOX_STATUS_EMPTY) || mmio_read(MMIO_BASE, MBOX_READ) != r) {
        }
    }
#endif

    uart_set_baudrate(UART_BAUD_RATE_1200);
    mmio_write(MMIO_BASE, UART0_IMSC,
               UART_INT_RIM | UART_INT_RX | UART_INT_RT | UART_INT_FE | UART_INT_PE | UART_INT_BE | UART_INT_OE);

    mmio_write(MMIO_BASE, AUX_ENABLES, AUX_ENABLES_MINI_UART);
    mmio_write(MMIO_BASE, UART1_IER_REG, UART1_IER_RX_INT);
}

void uart_putc(unsigned char c) {
    mmio_write(MMIO_BASE, UART0_IMSC, mmio_read(MMIO_BASE, UART0_IMSC) & ~UART_INT_TX);
    int was_empty = (tx_head == tx_tail);

    tx_buffer[tx_head] = c;
    tx_head = (1 + tx_head) % UART_BUFFER_SIZE;

    if (was_empty) {
        mmio_write(MMIO_BASE, UART0_DR, tx_buffer[tx_tail]); // ou mmio_write(UART0_DR, c);
        tx_tail = (1 + tx_tail) % UART_BUFFER_SIZE;
    }
    mmio_write(MMIO_BASE, UART0_IMSC, mmio_read(MMIO_BASE, UART0_IMSC) | UART_INT_TX);
}

void uart_puts(const char *str) {
    for (int i = 0; str[i] != '\0'; i++)
        uart_putc((unsigned char) str[i]);
}

void uart_put_hex(uint64_t hex) {
    uart_puts("0x");

    for (int i = 60; i >= 0; i -= 4) {
        // 0xF pour ne garder que les 4 bits les plus petits
        unsigned int nible = (hex >> i) & 0xF;
        if (nible < 10) {
            uart_putc((unsigned char) '0' + nible);
        } else {
            uart_putc((unsigned char) 'A' + nible - 10);
        }
    }
    uart_putc((unsigned char) '\n');
}

int uart_get_char_async() {
    if (rx_head != rx_tail) {
        char c = rx_buffer[rx_tail];
        rx_tail = (rx_tail + 1) % UART_BUFFER_SIZE;
        return c;
    }
    return -1;
}

void uart_handle_interrupt() {
    uint32_t mis = mmio_read(MMIO_BASE, UART0_MIS);
    if ((mis & UART_INT_RX) || (mis & UART_INT_RT)) {
        while (!(mmio_read(MMIO_BASE, UART0_FR) & UART_FR_RXFE)) {
            // 1. On récupère la touche (et ça acquitte l'interruption au niveau de l'UART)
            unsigned char c = mmio_read(MMIO_BASE, UART0_DR);

            if (c == '\r') {
                uart_puts("\r\n");
                shell_buffer[shell_index] = '\0';
                command_ready = true;
            } else if (c == '\b' || c == ASCII_DEL) {
                rx_buffer[shell_index] = '\0';
                if (shell_index > 0) {
                    shell_index--;
                    uart_puts("\b \b");
                }
            } else {
                uart_putc(c);
                shell_buffer[shell_index] = c;
                shell_index = (shell_index + 1) % SHELL_BUFFER_SIZE;
            }

            rx_buffer[rx_head] = c;
            rx_head = (rx_head + 1) % UART_BUFFER_SIZE;
        }
    }
    if (mis & UART_INT_TX) {
        if (tx_head != tx_tail) {
            mmio_write(MMIO_BASE, UART0_DR, tx_buffer[tx_tail]);
            tx_tail = (tx_tail + 1) % UART_BUFFER_SIZE;
        } else {
            mmio_write(MMIO_BASE, UART0_IMSC, mmio_read(MMIO_BASE, UART0_IMSC) & ~UART_INT_TX);
        }
    }
}

void execute_command() {
    if (!command_ready) return;

    shell_index = 0;
    char command[SHELL_BUFFER_SIZE];
    strcpy(command, shell_buffer);
    memset(shell_buffer, 0, SHELL_BUFFER_SIZE);
    command_ready = false;
    exec(command);

    print_shell();
}
