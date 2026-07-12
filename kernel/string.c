#include <kernel/string.h>

unsigned long strlen(const char *str) {
    unsigned long i = 0;
    while (str[i] != '\0') {
        i++;
    }

    return i;
}

bool strcmp(const char *str1, const char *str2) {
    unsigned long i = 0;
    while(str1[i] == str2[i]) {
        if (str1[i] == '\0') return true;
        i++;
    }

    return false;
}
