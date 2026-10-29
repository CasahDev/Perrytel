#pragma once
#include <stdbool.h>
#include <stdint.h>

enum fd_type{
    STDIN,
    STDOUT,
    FILE
};

typedef struct vfs_file {
    bool is_open;
    const struct vfs_driver *driver;
    enum fd_type type;
    uint64_t private_id;
    uint64_t size;
    uint64_t offset;
} vfs_file;

typedef struct vfs_driver {
    const char *name;
    int (*read_file)(const char *path, char *buffer, uint32_t max_size);
    int (*create_file)(const char *path, const char *content);
    int (*create_dir)(const char *path);
    int (*remove_file)(const char *path);
    int (*remove_dir)(const char *path);
    int (*list_dir)(const char *path, char ***out_file_list);
    int (*change_dir)(const char *path);
    int (*path)(char *out_path);
    bool (*file_exists)(const char *path);
    void (*open)(const char *path, vfs_file *out_file);
    uint64_t (*read)(vfs_file *file, char *buffer, uint64_t count);
} vfs_driver_t;

extern const struct vfs_driver *active_fs;