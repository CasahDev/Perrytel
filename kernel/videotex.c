#include <drivers/uart.h>

// 24 lignes, 40 colonnes (On initialise tout avec des espaces)
static char vram[24][40]; 

// Position actuelle du curseur "virtuel"
static int cursor_x = 1;
static int cursor_y = 1;

void minitel_init() {
    for (int x = 0; x < 40; x++) {
        for (int y = 0; y < 24; y++) {
            vram[y][x] = ' ';
        }
    }
}

void minitel_clear() {
    for (int x = 0; x < 40; x++) {
        for (int y = 0; y < 24; y++) {
            vram[y][x] = ' ';
        }
    }
}

void minitel_beep() {
    uart_putc(0x07);
}

void minitel_locate(int x, int y) {
    #ifdef QEMU
        uart_puts("\x1B[");
        if (y >= 10) uart_putc('0' + (y / 10)); // Dizaines de Y
        uart_putc('0' + (y % 10));              // Unités de Y
        uart_putc(';');
        if (x >= 10) uart_putc('0' + (x / 10)); // Dizaines de X
        uart_putc('0' + (x % 10));              // Unités de X
        uart_putc('H');
    #else
        uart_putc(0x1F);
        uart_putc(0x40 + y);
        uart_putc(0x40 + x);
    #endif
}

void minitel_print(const char *text) {
	for (int i = 0; text[i] != '\0'; i ++)
		minitel_putc((unsigned char)text[i]);
}

void minitel_cursor(int visible) {
    if (visible) {
        uart_putc(0x11);
    } else {
        uart_putc(0x14);
    }
}

void minitel_putc(char c) {
    if (c == '\n') {
        cursor_y++;
    } 
    else if (c == '\r') {
        cursor_x = 1;
    }
    else if (c >= 0x20 && c <= 0x7E) { 
        // On vérifie qu'on ne déborde pas de l'écran à droite
        if (cursor_x <= 40) {
            vram[cursor_y - 1][cursor_x - 1] = c;
            cursor_x++;
        }
    }
    
    // 3. Gestion du débordement de ligne (Word wrap)
    if (cursor_x > 40) {
        cursor_x = 1;
        cursor_y++;
    }
}

void minitel_removec() {
    if (cursor_x > 1) cursor_x--; vram[cursor_y - 1][cursor_x - 1] = ' ';
}

void minitel_flush() {
    minitel_cursor(0);
    for (int y = 0; y < 24; y++) {
        for (int x = 0; x < 40; x++) {
            uart_putc(vram[y][x]);
        }
        uart_putc('\r');
        uart_putc('\n');
    }

    minitel_locate(cursor_x, cursor_y + 1);

    minitel_cursor(1);

}