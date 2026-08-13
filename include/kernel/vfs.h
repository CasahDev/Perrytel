#pragma once

typedef struct vfs_driver {
    const char *name;
    int (*read_file)(const char *path, char *buffer, uint32_t max_size);
    int (*create_file)(const char *path, const char *content);
    int (*create_dir)(const char *path);
    // int (*remove_file)(const char *path);
    // int (*remove_dir)(const char *path);
    int (*list_dir)(const char *path, char ***out_file_list);
    int (*change_dir)(const char *path);
    // int (*parent_directory)();
    int (*path)(char *out_path);
} vfs_driver_t;

extern const struct vfs_driver *active_fs;