#include <drivers/fat32.h>
#include <drivers/sd.h>
#include <drivers/timer.h>
#include <drivers/uart.h>

#include <string.h>
#include <kernel/memory.h>
#include <kernel/shell.h>

struct vfs_driver driver;

void time_command(int argc, char **argv);
void help(int argc, char **argv);
void echo(int argc, char **argv);
void ls(int argc, char **argv);
void cat(int argc, char **argv);
void mkdir(int argc, char **argv);
void cd(int argc, char **argv);
void touch(int argc, char **argv);
void rm(int argc, char **argv);
void rmdir(int argc, char **argv);

static Command list[] = {
    {.name = "help", .run = help},
    {.name = "time", .run = time_command},
    {.name = "echo", .run = echo},
    {.name = "meminfo", .run = meminfo},
    {.name = "touch", .run = touch},
    {.name = "cat", .run = cat},
    {.name = "ls", .run = ls},
    {.name = "mkdir", .run = mkdir},
    {.name = "cd", .run = cd},
    {.name = "rm", .run = rm},
    {.name = "rmdir", .run = rmdir}
};

void help(int argc, char **argv) {
    uart_puts("Voici la liste des commandes disponibles\r\n");

    int command_number = sizeof(list) / sizeof(Command);
    for (int i = 0; i < command_number; i++) {
        uart_puts("- ");
        uart_puts(list[i].name);
        uart_puts("\r\n");
    }
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
    char **files = NULL;

    char *path = argc > 1 ? argv[1] : "";

    const uint32_t count = driver.list_dir(path, &files);

    for (uint32_t i = 0; i < count; i++) {
        uart_puts(files[i]);
        uart_puts("\r\n");
        kfree(files[i]);
    }

    kfree(files);
}

void cat(int argc, char **argv) {
    if (argc != 2) return;

    char content[2048];
    driver.read_file(argv[1], content, 2048);
    uart_puts(content);

    uart_puts("\r\n");
}

void touch(int argc, char **argv) {
    if (argc < 3) return;

    char *filename = argv[1];
    char *content = argv[2];
    driver.create_file(filename, content);
}

void mkdir(int argc, char **argv) {
    if (argc < 2) return;

    char *dir_name = argv[1];
    driver.create_dir(dir_name);
}

void cd(int argc, char **argv) {
    if (argc != 2) return;

    char *path = argv[1];
    driver.change_dir(path);
}

void rm(int argc, char **argv) {
    if (argc != 2) return;

    uint32_t status = driver.remove_file(argv[1]);
}

void rmdir(int argc, char **argv) {
    if (argc != 2) return;

    uint32_t status = driver.remove_dir(argv[1]);
}

void init_shell(struct vfs_driver storage_driver) {
    driver = storage_driver;
}

void exec(char *command) {
    char *argv[10];
    int argc = strsplit(command, argv, 10, ' ');

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
