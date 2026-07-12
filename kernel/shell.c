#include <kernel/shell.h>
#include <drivers/uart.h>
#include <kernel/string.h>
#include <drivers/timer.h>

const int command_number = 2;

void help(int argc, char **argv) {
    uart_puts("Voici la liste des commandes disponibles\r\n");
    uart_puts("help\r\n");
    uart_puts("time\r\n");
}

void time_command(int argc, char **argv) {
    char current[32];
    itoa(get_time(), current);
    uart_puts(current);
    uart_puts("\r\n");
}

Command list[] = {
    {"help", help},
    {"time", time_command}
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