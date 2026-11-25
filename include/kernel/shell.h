#pragma once

#include <kernel/vfs.h>

typedef void (*function)(int argc, char **argv);

typedef struct {
    const char *name;
    function run;
} Command;

void exec(char* command);

void init_shell(struct vfs_driver driver);

void print_shell();

void run_shell();