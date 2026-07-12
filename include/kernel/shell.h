typedef void (*function)();

typedef struct {
    const char *name;;
    function exec;
} Command;

void exec(const char* command);