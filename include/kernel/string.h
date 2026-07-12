#include <stdbool.h>

unsigned long strlen(const char *str);

bool strcmp(const char *str1, const char *str2);

void strcpy(const char *src, char *dest);

void itoa(int n, char *dist);

int strsplit(char *str, char **argv, int max_args);