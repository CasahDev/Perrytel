#include <drivers/fat32.h>
#include <drivers/sd.h>
#include <drivers/timer.h>
#include <drivers/uart.h>

#include <string.h>
#include <kernel/memory.h>
#include <kernel/shell.h>

struct vfs_driver driver;

void help(int argc, char **argv) {
    uart_puts("Voici la liste des commandes disponibles\r\n");
    uart_puts("help\r\n");
    uart_puts("time\r\n");
    uart_puts("echo\r\n");
    uart_puts("meminfo\r\n");
    uart_puts("ls\r\n");
    uart_puts("cat\r\n");
    uart_puts("touch\r\n");
    uart_puts("mkdir\r\n");
    uart_puts("cd\r\n");
    uart_puts("rm\r\n");
    uart_puts("rmdir\r\n");
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

        if (entry->attributes == LONG_FILE_NAME) {
            uart_puts("debug 2");
        }

        if ((uint8_t) entry->name[0] != EMPTY && (uint8_t) entry->name[0] != DELETED) {
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
        if ((uint8_t) entry->name[0] != EMPTY && (uint8_t) entry->name[0] != DELETED) {
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

    char *path = argv[1];
    driver.change_dir(path);
}

void rm(int argc, char **argv) {
    if (argc != 2) return;

    void *buffer = kalloc(512);
    sd_read_block(get_current_directory_sector(), buffer);

    const char *file_name = argv[1];

    for (int i = 0; i < 16; i++) {
        fat32_dir_entry *entry = &buffer[i * 32];

        if ((uint8_t) entry->name[0] != EMPTY && (uint8_t) entry->name[0] != DELETED) {
            void *file = kalloc(11);
            fat32_get_name(entry, file);

            if (strcmp(file, file_name) == 1) {
                if (entry->attributes == DIRECTORY) {
                    uart_puts("Impossible de supprimer un dossier");
                } else {
                    fat32_remove_file(entry, buffer, 0);
                }

                return;
            }
            kfree(file);
        }
    }

    kfree(buffer);
    uart_puts("\r\n");
}

void rmdir(int argc, char **argv) {
    if (argc != 2) return;

    void *buffer = kalloc(512);
    sd_read_block(get_current_directory_sector(), buffer);

    const char *file_name = argv[1];

    for (int i = 0; i < 16; i++) {
        fat32_dir_entry *entry = &buffer[i * 32];

        if ((uint8_t) entry->name[0] != EMPTY && (uint8_t) entry->name[0] != DELETED) {
            void *file = kalloc(11);
            fat32_get_name(entry, file);

            if (strcmp(file, file_name) == 1) {
                if (entry->attributes == DIRECTORY) {
                    fat32_remove_directory(entry, buffer, get_current_directory_sector());
                } else {
                    uart_puts("Ce n'est pas un dossier !");
                }

                return;
            }
            kfree(file);
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
    {.name = "mkdir", .run = mkdir},
    {.name = "touch", .run = touch},
    {.name = "rm", .run = rm},
    {.name = "rmdir", .run = rmdir}
};

void init_shell(struct vfs_driver storage_driver) {
    driver = storage_driver;
}

void exec(char *command) {
    char *argv[10];
    int argc = strsplit(command, argv, 10);

    int command_number = sizeof(list) / sizeof(Command);
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
    char path[128];
    driver.path(path);
    uart_puts("Perry (");
    uart_puts(path);
    uart_puts(") -> ");
}
