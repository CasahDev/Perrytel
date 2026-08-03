#include <drivers/fat32.h>
#include <drivers/sd.h>
#include <drivers/timer.h>
#include <drivers/uart.h>

#include <kernel/memory.h>
#include <kernel/shell.h>
#include <kernel/string.h>

const int command_number = 6;

void help(int argc, char **argv) {
    uart_puts("Voici la liste des commandes disponibles\r\n");
    uart_puts("help\r\n");
    uart_puts("time\r\n");
    uart_puts("echo\r\n");
    uart_puts("meminfo\r\n");
    uart_puts("ls\r\n");
}

void time_command(int argc, char **argv) {
    char current[32];
    itoa(get_time(), current);
    uart_puts(current);
    uart_puts("\r\n");
}

void echo(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        uart_puts(argv[i+1]);
        uart_putc(' ');
    } 

    uart_putc('\r');
    uart_putc('\n');
}

void ls(int argc, char **argv) {
    void *buffer = kalloc(512);
    sd_read_block(get_root_sector(), buffer);

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
    sd_read_block(get_root_sector(), buffer);

    for (int i = 0; i < 16; i++) {
        const fat32_dir_entry *entry = &buffer[i * 32];
        if ((uint8_t)entry->name[0] != 0x00 && (uint8_t)entry->name[0] != 0xE5) {
            void *file_name = kalloc(11);
            fat32_get_name(entry, file_name);
            if (strcmp(file_name, argv[1]) != 0) {
                char *content = fat32_read_file(entry);
                uart_puts(content);
                kfree(content);
                return;
            }
        }
    }

    kfree(buffer);
}

static Command list[] = {
    {.name = "help", .run = help},
    {.name = "time", .run = time_command},
    {.name = "echo", .run = echo},
    {.name = "meminfo", .run = meminfo},
    {.name = "ls", .run = ls},
    {.name = "cat", .run = cat}
};

void exec(char *command) {
    char *argv[10];
    int argc = strsplit(command, argv, 10);
    
    if (argc == 0) return; // Sécurité si on a juste tapé sur Entrée
    for (int i = 0; i < command_number; i++) {
        if (strcmp(argv[0], list[i].name)) {
            // Magie : on lance la fonction stockée dans la structure !
            list[i].run(argc, argv);
            return; // On a trouvé, on s'arrête
        }
    }

    uart_puts("Command introuvable\r\n");
}