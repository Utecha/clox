#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "vm.h"

#if defined(__unix__)
    #include <readline/readline.h>
    #include <readline/history.h>
#endif // __unix__

static int exitcode = 0;

static int get_exitcode()
{
    return exitcode;
}

static void set_exitcode(int value)
{
    exitcode = value;
}

static void repl(LoxVM *vm)
{
    #if !defined(__unix__)
        static char buffer[2048];
    #endif // __unix__

    for (;;)
    {
        #if defined(__unix__)
            char *line = readline(">>> ");
            if (!line)
            {
                printf("\n");
                return;
            }

            add_history(line);
            interpret_lox_vm(vm, line);
            free(line);
        #else
            printf(">>> ");
            if (!fgets(buffer, 2048, stdin))
            {
                printf("\n");
                return;
            }

            interpret_lox_vm(vm, buffer);
        #endif // __unix__
    }
}

static char *read_script(const char *filepath)
{
    FILE *file = fopen(filepath, "rb");
    if (!file)
    {
        fprintf(stderr, "Could not open file '%s'. Did you spell it right?\n", filepath);
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(file_size + 1);
    if (!buffer)
    {
        fprintf(stderr, "Not enough memory to read file '%s'\n", filepath);
        return NULL;
    }

    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    if (bytes_read < file_size)
    {
        fprintf(stderr, "Failed to read file '%s'\n", filepath);
        return NULL;
    }

    buffer[bytes_read] = '\0';
    fclose(file);
    return buffer;
}

static void run_script(LoxVM *vm, const char *filepath)
{
    char *source = read_script(filepath);
    if (!source)
    {
        set_exitcode(65);
        return;
    }

    InterpretResult result = interpret_lox_vm(vm, source);
    free(source);

    if (result == RESULT_COMPILE_ERROR) set_exitcode(65);
    if (result == RESULT_RUNTIME_ERROR) set_exitcode(70);
}

int main(int argc, char **argv)
{
    LoxVM vm;
    init_lox_vm(&vm);

    switch (argc)
    {
        case 1: repl(&vm); break;
        case 2: run_script(&vm, argv[1]); break;
        default:
            fprintf(stderr, "Usage: lox [script]");
            return 64;
    }

    free_lox_vm(&vm);
    return get_exitcode();
}
