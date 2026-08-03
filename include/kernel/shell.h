#pragma once
typedef void (*function)(int argc, char **argv);

typedef struct {
    const char *name;
    function run;
} Command;

void exec(char* command);
