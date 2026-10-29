#pragma once
#include <stdbool.h>
#include <kernel/memory.h>

unsigned long strlen(const char *str);

bool strcmp(const char *str1, const char *str2);

void strcpy(char *dest, const char *src);

void itoa(int n, char *dist);

int strsplit(char *str, char **argv, int max_args, char splitter);

int strcat(char *str, const char *str1, const char *str2);

char to_lower(char c);

void strlow(char *dest, const char *src);

int strncmp(const char *str1, const char *str2, int len);