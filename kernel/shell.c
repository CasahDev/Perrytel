#include <drivers/fat32.h>
#include <drivers/sd.h>
#include <drivers/timer.h>
#include <drivers/uart.h>

#include <string.h>
#include <kernel/memory.h>
#include <kernel/shell.h>
#include <kernel/user.h>

#include "lwip/netif.h"
#include "lwip/ip_addr.h"

#include <errors.h>

#include "netif/slipif.h"
#include "lwip/timeouts.h"

// Maximum stack size for the user mode
#define USER_STACK_SIZE 4096


struct vfs_driver driver;

const char bin_folder[6] = "/bin/";

void *current_user_stack = 0;

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
void ifconfig(int argc, char **argv);

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
    {.name = "rmdir", .run = rmdir},
    {.name = "ifconfig", .run = ifconfig}
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

void ifconfig(int argc, char **argv) {
    (void)argc;
    (void)argv;

    struct netif *net = netif_default;

    if (net == 0) {
        uart_puts("no default interfaces !");
        return;
    }

    uart_puts("--------------\n");
    uart_puts(net->name);

    if (netif_is_up(net)) {
        uart_puts(" UP");
    } else {
        uart_puts(" DOWN");
    }
    uart_puts("\r\n");

    char inet[21];
    strcat(inet, "inet ", ip4addr_ntoa(netif_ip4_addr(net)));
    uart_puts(inet);
    uart_puts("\r\n");

    char mask[21];
    strcat(mask, "mask ", ip4addr_ntoa(netif_ip4_netmask(net)));
    uart_puts(mask);
    uart_puts("\r\n");

    char gw[24];
    strcat(gw, "gateway ", ip4addr_ntoa(netif_ip4_gw(net)));
    uart_puts(gw);
    uart_puts("\r\n");
}

void init_shell(const struct vfs_driver storage_driver) {
    driver = storage_driver;
}

void exec(char *command) {
    char *argv[10];
    int argc = strsplit(command, argv, 10, ' ');

    int command_number = sizeof(list) / sizeof(Command);
    if (argc == 0) return;
    for (int i = 0; i < command_number; i++) {
        if (streq(argv[0], list[i].name) != 0) {
            list[i].run(argc, argv);
            return; // On a trouvé, on s'arrête
        }
    }

    const char *command_name = argv[0];
    char bin[64];

    strcat(bin, bin_folder, command_name);

    if (driver.file_exists(bin)) {
        current_user_stack = kalloc(USER_STACK_SIZE);

        void *stack_top = current_user_stack + USER_STACK_SIZE;

        //load file
        const int32_t fd = driver.read_file(bin, (char *) USER_PROGRAM_BASE, USER_BIN_MAX_SIZE);
        if (fd == FILE_NOT_FOUND) {
            uart_puts("Command not found");
            return;
        }
        switch_to_user_mode((void *) USER_PROGRAM_BASE, stack_top);
    }
}

void print_shell() {
    char path[128];
    driver.path(path);
    uart_puts("Perry (");
    uart_puts(path);
    uart_puts(") -> ");
}

void run_shell() {
    print_shell();

    while (true) {
        __asm volatile("wfi");
        if (is_command_ready()) {
            execute_command();
        }

        sys_check_timeouts();
        if (netif_default != NULL) slipif_poll(netif_default);
    }
}