void minitel_clear(void);

void minitel_beep(void);

void minitel_locate(int x, int y);

void minitel_print(const char *text);

void minitel_putc(char c);

void minitel_cursor(int visible);

void minitel_flush(void);
void minitel_hardware_clear(void);

void minitel_draw_image(int start_x, int start_y, int width, int height, const int *logo);