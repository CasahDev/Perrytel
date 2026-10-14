//
// Created by sacha on 26/08/2026.
//

#include <stdint.h>
#include <drivers/uart.h>

#include "syscall_internal.h"

#include <kernel/syscall.h>

#define FD_NB 32

vfs_file fd_list[FD_NB];

void init_fd() {
    for (int i = 0; i < FD_NB; i++) {
        fd_list[i].is_open = false;
    }

    fd_list[0].is_open = true;
    fd_list[0].driver = &uart_vfs_driver;
}

static uint32_t get_free_fd() {
    uint32_t result = -1;
    uint32_t fd = 0;
    const uint32_t entry_number = sizeof(fd_list) / sizeof(vfs_file);
    while (fd < entry_number) {
        if (!fd_list[fd].is_open) {
            result = fd;
            break;
        }
        fd++;
    }

    return result;
}

uint32_t sys_open(const uint32_t syscall_id, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    const char *path = (char *) arg0;

    const uint8_t fd = get_free_fd();

    const bool exists = active_fs->file_exists(path);

    if (!exists) return -1;

    fd_list[fd].type = FILE;
    fd_list[fd].offset = 0;
    fd_list[fd].is_open = false;

    active_fs->open(path, &fd_list[fd]);


    return fd;
}

uint32_t sys_read(const uint32_t syscall_id, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    uint32_t fd = arg0;
    char *buffer = (char *) arg1;
    uint64_t count = arg2;

    vfs_file *file = &fd_list[fd];
    return file->driver->read(file, buffer, count);
}