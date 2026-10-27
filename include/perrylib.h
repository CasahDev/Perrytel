#pragma once
#include <stdint.h>
#include <errors.h>

int64_t readf(int fd, char *out, uint64_t buffer_size);
void printf(char *str);
void exit(uint32_t code);

int64_t open(char *file);
int64_t close(uint64_t fd);