#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/history.h>
#include <readline/readline.h>
#include "include/vm.h"

/*
 * NOTE: Readline is included AFTER stdio because of the
 * need for the 'FILE' type.
*/

static void repl() {
    for (;;) {
        char *line = readline(">>> ");
        if (line == NULL) {
            printf("\n");
            clear_history();
            return;
        }

        add_history(line);
        interpret(line);
        free(line);
    }
}

static char *read_file(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file '%s'. Did you spell it right?\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(file_size + 1);
    if (file == NULL) {
        fprintf(stderr, "Not enough memory to read '%s'\n", path);
        exit(74);
    }

    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    if (bytes_read < file_size) {
        fprintf(stderr, "Could not read file '%s'", path);
        exit(74);
    }

    buffer[bytes_read] = '\0';
    fclose(file);
    return buffer;
}

static void run_file(const char *path)
{
    char *source = read_file(path);
    VMResult result = interpret(source);
    free(source);

    if (result == VM_COMPILE_ERROR) exit(65);
    if (result == VM_RUNTIME_ERROR) exit(70);
}

int main(int argc, char **argv)
{
    init_vm();

    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        run_file(argv[1]);
    } else {
        fprintf(stderr, "Usage: clox [script.lox]");
        exit(64);
    }

    free_vm();
    return 0;
}
