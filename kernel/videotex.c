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
    else if ((c >= 0x20 && c <= 0x7F) || c == 0x0E || c == 0x0F) {                   
        vram[cursor_y - 1][cursor_x - 1] = c;
        cursor_x++;
        screen_dirty = 1;
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

void minitel_put_mosaic(char c) {
    // Traduction conditionnelle
    #ifdef QEMU
        int m_pixel = (c >= 0x60) ? (c - 0x40) :  (c - 0x20);                                                                   
        uart_put_braille(minitel_to_braille(m_pixel));
    #else
        uart_putc(c);
    #endif
}

void minitel_flush_char(int x, int y, int is_mosaic) {
    // Si la case n'a pas changé, on quitte la fonction immédiatement !
    if (previous_vram[y][x] == vram[y][x]) {
        return; 
    }

    minitel_locate(x + 1, y + 1); // Toujours se positionner
        
    if (vram[y][x] == 0x0E || vram[y][x] == 0x0F) {
        uart_putc(' '); // L'attribut matériel est invisible (espace)
    }                                                                            
    else if (is_mosaic) {
        minitel_put_mosaic(vram[y][x]);
    }         
    else {
        // Texte normal
        uart_putc(vram[y][x]);
    }

    // On n'oublie pas de sauvegarder la VRAM
    previous_vram[y][x] = vram[y][x];    
}

void minitel_flush() {
    if (!screen_dirty) return;

    minitel_cursor(0);
    for (int y = 0; y < 24; y++) {
        int is_mosaic = 0;
        for (int x = 0; x < 40; x++) {
            if (vram[y][x] == 0x0E) is_mosaic = 1;
            else if (vram[y][x] == 0x0F) is_mosaic = 0;
        
            // 2. S'il y a un changement, on dessine !
            minitel_flush_char(x, y, is_mosaic);
        } 
    }
    
    minitel_locate(cursor_x, cursor_y);
    
    minitel_cursor(1);
    screen_dirty = 0;
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
            // On force le curseur virtuel de l'OS au bon endroit !
            cursor_x = start_x;
            cursor_y = start_y + y;
            
            minitel_putc(0x0E); // Active la mosaïque (prend la case X=1)
            
            // On saute la première et dernière colonne de l'image (X=1 à width-1)       
            // pour que le total (0x0E + Image + 0x0F) ne dépasse pas 40 colonnes !      
            for (int x = 1; x < width - 1; x++) {
                minitel_draw_block(logo[y * width + x]); 
            }
            
            minitel_putc(0x0F); // Désactive la mosaïque (prend la case X=40)            
        }
    }

void minitel_draw_block(int pixels) {
    if (pixels > 31 && pixels < 64) {
        minitel_putc(0x40 + pixels);
    } else if (pixels >= 0) {
        minitel_putc(0x20 + pixels);
    }
}

int minitel_to_braille(int m_pixels) {
    int b = 0;
    if (m_pixels & 1)  b += 1;  // Haut-Gauche : même chose
    if (m_pixels & 2)  b += 8;  // Haut-Droit : vaut 2 en Minitel, mais 8 en Braille     
    if (m_pixels & 4)  b += 2;  // Milieu-Gauche
    if (m_pixels & 8)  b += 16;  // Haut-Gauche : même chose
    if (m_pixels & 16)  b += 4;  // Haut-Droit : vaut 2 en Minitel, mais 8 en Braille     
    if (m_pixels & 32)  b += 32;  // Milieu-Gauche
    return b;
}

void uart_put_braille(int b_pixels) {
        uart_putc(0xE2);
        uart_putc(0xA0);
        uart_putc(0x80 + b_pixels); // Magie noire UTF-8
}