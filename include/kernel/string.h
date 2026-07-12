#include <stdbool.h>

unsigned long strlen(const char *str);

bool strcmp(const char *str1, const char *str2);

void strcpy(char *dest, const char *src);

void itoa(int n, char *dist);

int strsplit(char *str, char **argv, int max_args);