#include <drivers/fat32.h>
#include <drivers/sd.h>
#include <drivers/timer.h>
#include <drivers/uart.h>

#include <kernel/memory.h>
#include <kernel/shell.h>
#include <string.h>

const int command_number = 9;
int parent_directory = 2;
static char current_path[128] = "/";

void help(int argc, char **argv) {
    uart_puts("Voici la liste des commandes disponibles\r\n");
    uart_puts("help\r\n");
    uart_puts("time\r\n");
    uart_puts("echo\r\n");
    uart_puts("meminfo\r\n");
    uart_puts("ls\r\n");
    uart_puts("cat\r\n");
    uart_puts("touch\r\n");
}

void time_command(int argc, char **argv) {
    char current[32];
    itoa(get_time(), current);
    uart_puts(current);
    uart_puts("\r\n");
}

void echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        uart_puts(argv[i]);
        uart_putc(' ');
    } 

    uart_putc('\r');
    uart_putc('\n');
}

void ls(int argc, char **argv) {
    void *buffer = kalloc(512);
    sd_read_block(get_current_directory_sector(), buffer);

    for (int i = 0; i < 16; i++) {
        const fat32_dir_entry *entry = &buffer[i * 32];

        if (entry->attributes == 0x0F) {
            uart_puts("debug 2");
        }

        if ((uint8_t)entry->name[0] != 0x00 && (uint8_t)entry->name[0] != 0xE5) {
            void *file_name = kalloc(11);
            fat32_get_name(entry, file_name);
            uart_puts(file_name);
            kfree(file_name);
            uart_puts("\r\n");
        }
    }

    kfree(buffer);
}

void cat(int argc, char **argv) {
    if (argc != 2) return;

    void *buffer = kalloc(512);

    sd_read_block(get_current_directory_sector(), buffer);

    const char *file = argv[1];

    for (int i = 0; i < 16; i++) {
        const fat32_dir_entry *entry = &buffer[i * 32];
        if ((uint8_t)entry->name[0] != 0x00 && (uint8_t)entry->name[0] != 0xE5) {
            void *file_name = kalloc(11);
            fat32_get_name(entry, file_name);
            if (strcmp(file_name, file) != 0) {
                char *content = fat32_read_file(entry);
                uart_puts(content);
                kfree(content);
                kfree(file_name);
                kfree(buffer);
                return;
            }
            kfree(file_name);
        }
    }

    kfree(buffer);
    uart_puts("\r\n");
}

void touch(int argc, char **argv) {
    if (argc < 3) return;

    char *filename = argv[1];
    char *content = argv[2];
    fat32_create_file(filename, content);
}

void mkdir(int argc, char **argv) {
    if (argc < 2) return;

    char *dir_name = argv[1];
    fat32_create_directory(dir_name);
}

void cd(int argc, char **argv) {
    if (argc != 2) return;

    void *buffer = kalloc(512);
    sd_read_block(get_current_directory_sector(), buffer);

    const char *directory = argv[1];

    for (int i = 0; i < 16; i++) {

        const fat32_dir_entry *entry = &buffer[i * 32];

        if ((uint8_t)entry->name[0] != 0x00 && (uint8_t)entry->name[0] != 0xE5) {
            void *dir = kalloc(11);
            fat32_get_name(entry, dir);

            if (strcmp(dir, argv[1]) == 1) {
                if (entry->attributes == 0x10) {
                    uint32_t target_cluster = (entry->cluster_entry_high << 16) | entry->cluster_entry_low;
                    if (target_cluster == 0) {
                        target_cluster = 2;
                        memset(current_path, ' ', 128);
                    }

                    if (strcmp(directory, "..") == 1) {
                        uint8_t c = strlen(current_path) - 1;
                        while (current_path[c] != '/' && c > 0) {
                            c--;
                        }

                        if (c == 0) {
                            current_path[0] = '/';
                            current_path[1] = '\0';
                        } else {
                            current_path[c] = '\0';
                        }
                    } else {
                        if (current_path[strlen(current_path)-1] != '/') {
                            current_path[strlen(current_path)] = '/';
                        }
                        for (uint8_t l = 0; l < strlen(directory); l++) {
                            current_path[strlen(current_path)] = directory[l];
                        }
                        current_path[strlen(current_path)] = '\0';
                    }

                    set_current_directory(target_cluster);
                } else {
                    uart_puts("Ce n'est pas un dossier");
                }
            }
            kfree(dir);
        }
    }

    kfree(buffer);
    uart_puts("\r\n");
}

static Command list[] = {
    {.name = "help", .run = help},
    {.name = "time", .run = time_command},
    {.name = "echo", .run = echo},
    {.name = "meminfo", .run = meminfo},
    {.name = "ls", .run = ls},
    {.name = "cat", .run = cat},
    {.name = "cd", .run = cd},
    {.name = "mkdir", .run = mkdir}
};

void exec(char *command) {
    char *argv[10];
    int argc = strsplit(command, argv, 10, ' ');
    
    if (argc == 0) return; // Sécurité si on a juste tapé sur Entrée
    for (int i = 0; i < command_number; i++) {
        if (strcmp(argv[0], list[i].name) != 0) {
            // Magie : on lance la fonction stockée dans la structure !
            list[i].run(argc, argv);
            return; // On a trouvé, on s'arrête
        }
    }

    uart_puts("Command introuvable\r\n");
}

void print_shell() {
    uart_puts("Perry (");
    uart_puts(current_path);
    uart_puts(") -> ");
}