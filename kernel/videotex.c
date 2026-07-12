#include <drivers/uart.h>

// 24 lignes, 40 colonnes (On initialise tout avec des espaces)
static char vram[24][40]; 
static char previous_vram[24][40]; 

// Position actuelle du curseur "virtuel"
static int cursor_x = 1;
static int cursor_y = 1;

static int screen_dirty = 1;

void minitel_init() {
    for (int x = 0; x < 40; x++) {
        for (int y = 0; y < 24; y++) {
            vram[y][x] = ' ';
            previous_vram [y][x] = ' ';
        }
    }
}

void minitel_clear() {
    for (int x = 0; x < 40; x++) {
        for (int y = 0; y < 24; y++) {
            vram[y][x] = ' ';
        }
    }

    screen_dirty = 1;
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
    else if ((c >= 0x20 && c <= 0x7E) || c == 0x0E || c == 0x0F) {         
        // On vérifie qu'on ne déborde pas de l'écran à droite
        if (cursor_x <= 40) {
            vram[cursor_y - 1][cursor_x - 1] = c;
            cursor_x++;
            screen_dirty = 1;
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
    if (screen_dirty) {
        minitel_cursor(0);
        for (int y = 0; y < 24; y++) {
            for (int x = 0; x < 40; x++) {
                if (previous_vram[y][x] != vram[y][x]) {
                    minitel_locate(x + 1, y + 1);
                    uart_putc(vram[y][x]);
                    previous_vram[y][x] = vram[y][x];
                }
            }
        }

        minitel_locate(cursor_x, cursor_y);

        minitel_cursor(1);
        screen_dirty = 0;
    }
}

void minitel_hardware_clear() {
    #ifdef QEMU
        uart_puts("\x1B[2J\x1B[H");
        uart_putc('\r');
        uart_putc('\n');
    #else
        uart_putc(0x0C);
    #endif
}

void minitel_draw_image(int start_x, int start_y, int width, int height, const int *logo) {
    for (int y = 0; y < height; y++) {
        minitel_locate(start_x, start_y + y); // On va au début de la ligne          
        minitel_putc(0x0E); // On allume le mode dessin
        
        for (int x = 0; x < width; x++) {
            minitel_draw_block(logo[y * width + x]); // Formule 2D vers 1D           
        }
        
        minitel_putc(0x0F); // On remet le texte normal
    }
}

void minitel_draw_block(int pixels) {
    if (pixels > 31 && pixels < 64) {
        minitel_putc(0x40 + pixels);
    } else if (pixels >= 0) {
        minitel_putc(0x20 + pixels);
    }
}