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

int strsplit(char *str, char **argv, int max_args) {
    bool in_quote = false;
    unsigned long i = 0;
    int argc = 0;
    while (str[i] != '\0') {
        if (str[i] == '"') {
            in_quote = !in_quote;
            str[i] = '\0';
            if (in_quote) {
                argv[argc] = &str[i + 1];
                argc++;
            }
            i++;
        }
        else if (str[i] == ' ' && !in_quote) str[i] = '\0';
        else if(i == 0 || str[i-1] == '\0'){
             argv[argc] = &str[i];
             argc++;
        }
        i++;
    }

    return argc;
}