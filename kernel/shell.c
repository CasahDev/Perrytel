#include <kernel/shell.h>
#include <drivers/uart.h>
#include <kernel/string.h>

const int command_number = 1;

void help() {
    uart_puts("Voici la liste des commandes disponibles\r\n");
    uart_puts("help\r\n");
}

Command list[] = {
    {"help", help}
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