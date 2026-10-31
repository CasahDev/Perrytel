#include <string.h>

unsigned long strlen(const char *str) {
    unsigned long i = 0;
    while (str[i] != '\0') {
        i++;
    }

    return i;
}

bool streq(const char *str1, const char *str2) {
    unsigned long i = 0;
    while(str1[i] == str2[i]) {
        if (str1[i] == '\0') return true;
        i++;
    }

    return false;
}

void strcpy(char *dest, const char *src) {
    unsigned long i = 0; 
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

void itoa(int n, char *dist) {
    if (n == 0) {
        dist[0] = '0'; 
        dist[1] = '\0';
        return;
    }

    bool is_negative = false;
    if (n < 0) {
        is_negative = true;
        dist[0] = '-';
        n = -n;
    }
    
    int i = 0;
    while (n != 0) {
        dist[i] = 48 + (n % 10);
        i++;
        n /= 10;
    }

    if (is_negative) {
        dist[i] = '-';
        i++;
    }

    for (int start = 0; start < i/2; start++) {
        char temp = dist[start];
        dist[start] = dist[i-start-1];
        dist[i-start-1] = temp;
    }

    dist[i] = '\0';
}

int strsplit(char *str, char **out, const uint32_t max_args, const char splitter) {
    bool in_quote = false;
    uint32_t i = 0;
    int argc = 0;
    while (str[i] != '\0' && i < max_args) {
        if (str[i] == '"') {
            in_quote = !in_quote;
            str[i] = '\0';
            if (in_quote) {
                out[argc] = &str[i + 1];
                argc++;
            }
            i++;
        }
        else if (str[i] == splitter && !in_quote) str[i] = '\0';
        else if(i == 0 || str[i-1] == '\0'){
             out[argc] = &str[i];
             argc++;
        }
        i++;
    }

    return argc;
}

int strcat(char *str, const char *str1, const char *str2) {
    strcpy(str, str1);
    strcpy(str + strlen(str1), str2);

    return 0;
}

char to_lower(const char c) {
    if (c < 91 && c > 64) {
        return (char) ((int) c + 32);
    }

    return c;
}

void strlow(char *dest, const char *src) {
    for (uint64_t i = 0; i < strlen(src); i++) {
        dest[i] = to_lower(src[i]);
    }
}

int strncmp(const char *str1, const char *str2, const int len) {
    int i = 0;
    while (i < len) {
        if (str1[i] != str2[i]) return str1[i] - str2[i];
        if (str1[i] == '\0') return 0;
        i++;
    }

    return 0;
}

long strtol(const char *str, char **endptr, int base) {
    (void)base;

    long res = 0;
    uint32_t i = 0;

    while (str[i] == ' ') i++;

    while ( str[i] >= '0' && str[i] <= '9') {
        res = res * 10 + (str[i] - '0');
        i++;
    }

    if (endptr != 0) *endptr = (char *) str + i;

    return res;
}