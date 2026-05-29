#include <drivers/uart.h>
#include <stdint.h>

static uint32_t MMIO_BASE;

// Memory-Mapped I/O output
static inline void mmio_write(uint32_t reg, uint32_t data)
{
	*(volatile uint32_t*)(uintptr_t)(MMIO_BASE + reg) = data;
}

// Memory-Mapped I/O input
static inline uint32_t mmio_read(uint32_t reg)
{
	return *(volatile uint32_t*)(uintptr_t)(MMIO_BASE + reg);
}

// Loop <delay> times in a way that the compiler won't optimize away
static inline void delay(int32_t count)
{
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}

enum
{
    // The offsets for reach register.
    GPIO_BASE = 0x200000,
    GPFSEL1 = (GPIO_BASE + 0x04),
    GPPUD = (GPIO_BASE + 0x94),
    GPPUDCLK0 = (GPIO_BASE + 0x98),
    UART0_BASE = (GPIO_BASE + 0x1000),
    UART0_DR     = (UART0_BASE + 0x00),
    UART0_RSRECR = (UART0_BASE + 0x04),
    UART0_FR     = (UART0_BASE + 0x18),
    UART0_ILPR   = (UART0_BASE + 0x20),
    UART0_IBRD   = (UART0_BASE + 0x24),
    UART0_FBRD   = (UART0_BASE + 0x28),
    UART0_LCRH   = (UART0_BASE + 0x2C),
    UART0_CR     = (UART0_BASE + 0x30),
    UART0_IMSC   = (UART0_BASE + 0x38),
    UART0_ICR    = (UART0_BASE + 0x44),
    MBOX_BASE    = 0xB880,
    MBOX_STATUS  = (MBOX_BASE + 0x18),
    MBOX_WRITE   = (MBOX_BASE + 0x20),
    MBOX_READ    = (MBOX_BASE + 0x00)
};

volatile unsigned int __attribute__((aligned(16))) mbox[9] = {
    9*4, 0, 0x38002, 12, 8, 2, 3000000, 0 ,0
};

void uart_init(int raspi)
{
    switch (raspi) {
        case 2:
        case 3:  MMIO_BASE = 0x3F000000; break;
        case 4:  MMIO_BASE = 0xFE000000; break;
        default: MMIO_BASE = 0x20000000; break;
    }

    mmio_write(UART0_CR, 0x00000000);

    uint32_t selector = mmio_read(GPFSEL1);
    selector &= ~((7 << 12) | (7 << 15));
    selector |= (4 << 12) | (4 << 15);
    mmio_write(GPFSEL1, selector);

    mmio_write(GPPUD, 0x00000000);
    delay(150);
    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);
    mmio_write(GPPUDCLK0, 0x00000000);
    mmio_write(UART0_ICR, 0x7FF);

    #ifndef QEMU
    if (raspi >= 3) {
        unsigned int r = (((uintptr_t)(&mbox) & ~0xF) | 8);
        while ( mmio_read(MBOX_STATUS) & 0x80000000 ) { }
        mmio_write(MBOX_WRITE, r);
        while ( (mmio_read(MBOX_STATUS) & 0x40000000) || mmio_read(MBOX_READ) != r ) { }
    }
	#endif

    mmio_write(UART0_IBRD, 1);
    mmio_write(UART0_FBRD, 40);
    mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));
    mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
                           (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));
    mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void uart_putc(unsigned char c)
{
	while ( mmio_read(UART0_FR) & (1 << 5) ) { }
	mmio_write(UART0_DR, c);
}

unsigned char uart_getc()
{
    while ( mmio_read(UART0_FR) & (1 << 4) ) { }
    return mmio_read(UART0_DR);
}

void uart_puts(const char* str)
{
	for (int i = 0; str[i] != '\0'; i ++)
		uart_putc((unsigned char)str[i]);
}

void uart_put_hex(uint64_t hex) {
    uart_puts("0x");

    for (int i = 60; i < 0; i -= 4) {
        // 0xF pour ne garder que les 4 bits les plus petits
        unsigned int nible = (hex >> i) & 0xF;
        if (nible < 10) {
            uart_putc((unsigned char) '0' + nible);
        } else {
            uart_putc((unsigned char) 'A' + hex - 10);
        }
    }
    uart_putc((unsigned char) '\n');
}