#include <stdbool.h>
#include <stdint.h>

#include <drivers/uart.h>

#include <kernel/memory.h>
#include <kernel/mmio.h>
#include <kernel/shell.h>

static uint32_t MMIO_BASE;

volatile unsigned char tx_buffer[1024];
volatile uint32_t tx_head;
volatile uint32_t tx_tail;

volatile unsigned char rx_buffer[1024];
volatile uint32_t rx_head;
volatile uint32_t rx_tail;

char shell_buffer[256];
volatile int shell_index;
volatile bool command_ready = false;

// Loop <delay> times in a way that the compiler won't optimize away
static inline void delay(int32_t count)
{
	__asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}

volatile unsigned int __attribute__((aligned(16))) mbox[9] = {
    9*4, 0, 0x38002, 12, 8, 2, 3000000, 0 ,0
};

void uart_set_baudrate(uint32_t baud) {
    mmio_write(MMIO_BASE, UART0_CR, 0);
    uint32_t divisor = 12000000 / baud;
    uint32_t ibrd = divisor / 64;
    uint32_t fbrd = divisor % 64;
    mmio_write(MMIO_BASE, UART0_IBRD, ibrd);
    mmio_write(MMIO_BASE, UART0_FBRD, fbrd);
    mmio_write(MMIO_BASE, UART0_LCRH, (1 << 1) | (1 << 2) | (1 << 4) | (1 << 6));
    mmio_write(MMIO_BASE, UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void uart_init(int raspi)
{
    switch (raspi) {
        case 2:
        case 3:  MMIO_BASE = 0x3F000000; break;
        case 4:  MMIO_BASE = 0xFE000000; break;
        default: MMIO_BASE = 0x20000000; break;
    }

    mmio_write(MMIO_BASE, UART0_CR, 0x00000000);

    uint32_t selector = mmio_read(MMIO_BASE, GPFSEL1);
    selector &= ~((7 << 12) | (7 << 15));
    selector |= (4 << 12) | (4 << 15);
    mmio_write(MMIO_BASE, GPFSEL1, selector);

    mmio_write(MMIO_BASE, GPPUD, 0x00000000);
    delay(150);
    mmio_write(MMIO_BASE, GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);
    mmio_write(MMIO_BASE, GPPUDCLK0, 0x00000000);
    mmio_write(MMIO_BASE, UART0_ICR, 0x7FF);

    #ifndef QEMU
    if (raspi >= 3) {
        unsigned int r = (((uintptr_t)(&mbox) & ~0xF) | 8);
        while ( mmio_read(MMIO_BASE, MBOX_STATUS) & 0x80000000 ) { }
        mmio_write(MMIO_BASE, MBOX_WRITE, r);
        while ( (mmio_read(MMIO_BASE, MBOX_STATUS) & 0x40000000) || mmio_read(MMIO_BASE, MBOX_READ) != r ) { }
    }
	#endif

    uart_set_baudrate(1200);
    mmio_write(MMIO_BASE, UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 6) |
                           (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));
}

void uart_putc(unsigned char c)
{
    mmio_write(MMIO_BASE, UART0_IMSC, mmio_read(MMIO_BASE, UART0_IMSC) & ~(1 << 5));
    int was_empty = (tx_head == tx_tail);

    tx_buffer[tx_head] = c;
    tx_head = (1 + tx_head) % 1024;

    if (was_empty) {
        mmio_write(MMIO_BASE, UART0_DR, tx_buffer[tx_tail]); // ou mmio_write(UART0_DR, c);
        tx_tail = (1 + tx_tail) % 1024;
    }
   mmio_write(MMIO_BASE, UART0_IMSC, mmio_read(MMIO_BASE, UART0_IMSC) | (1 << 5));
}

unsigned char uart_getc()
{
    while ( mmio_read(MMIO_BASE, UART0_FR) & (1 << 4) ) { }
    return mmio_read(MMIO_BASE, UART0_DR);
}

void uart_puts(const char* str)
{
	for (int i = 0; str[i] != '\0'; i ++)
		uart_putc((unsigned char)str[i]);
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
        rx_tail = (rx_tail + 1) % 1024;
        return c;
    }
    return -1;
}

void uart_handle_interrupt() {
    uint32_t mis = mmio_read(MMIO_BASE, UART0_MIS);
    if ((mis & (1 << 4)) || (mis & (1 << 6))) {
        while(!(mmio_read(MMIO_BASE, UART0_FR) & (1 << 4))) {
            // 1. On récupère la touche (et ça acquitte l'interruption au niveau de l'UART)
            unsigned char c = mmio_read(MMIO_BASE, UART0_DR);

            if (c == '\r') {
                uart_putc('\r');
                uart_putc('\n');
                shell_buffer[shell_index] = '\0';
                command_ready = true;
                execute_command();
            } else {
                uart_putc(c);
                shell_buffer[shell_index] = c;
                shell_index = (shell_index + 1) % 256;
            }

            rx_buffer[rx_head] = c;
            rx_head = (rx_head + 1) % 1024;
        }
    }
    if (mis & (1 << 5) ) {
        if (tx_head != tx_tail) {
            mmio_write(MMIO_BASE, UART0_DR, tx_buffer[tx_tail]);
            tx_tail = (tx_tail + 1) % 1024;
        } else {
            mmio_write(MMIO_BASE, UART0_IMSC, mmio_read(MMIO_BASE, UART0_IMSC) & ~(1 << 5));
        }
    }
}

void execute_command() {
    if (!command_ready) return;

    exec(shell_buffer);

    shell_index = 0;
    command_ready = false;
    memset(shell_buffer, 0, 256);

    print_shell();
}