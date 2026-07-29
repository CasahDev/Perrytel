#include <kernel/shell.h>
#include <drivers/uart.h>
#include <kernel/string.h>
#include <drivers/timer.h>
#include <kernel/memory.h>

const int command_number = 4;

void help(int argc, char **argv) {
    uart_puts("Voici la liste des commandes disponibles\r\n");
    uart_puts("help\r\n");
    uart_puts("time\r\n");
    uart_puts("echo\r\n");
    uart_puts("meminfo\r\n");
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

static Command list[] = {
    {.name = "help", .run = help},
    {.name = "time", .run = time_command},
    {.name = "echo", .run = echo},
    {.name = "meminfo", .run = meminfo}
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