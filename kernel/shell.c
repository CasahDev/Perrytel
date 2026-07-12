#include <kernel/shell.h>
#include <drivers/uart.h>
#include <kernel/string.h>
#include <drivers/timer.h>

const int command_number = 2;

void help() {
    uart_puts("Voici la liste des commandes disponibles\r\n");
    uart_puts("help\r\n");
    uart_puts("time\r\n");
}

void time_command() {
    char *current;
    itoa(get_time(), current);
    uart_puts(current);
    uart_puts("\r\n");
}

Command list[] = {
    {"help", help},
    {"time", time_command}
};

void exec(const char *command) {
    for (int i = 0; i < command_number; i++) {
        if (strcmp(command, list[i].name)) {
            // Magie : on lance la fonction stockée dans la structure !
            list[i].exec();
            return; // On a trouvé, on s'arrête
        }
    }

    uart_puts("Command introuvable\r\n");
}