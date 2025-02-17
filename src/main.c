#include <stdio.h>
#include <stdlib.h>
#include "internal.h"
#include "vm.h"

#if defined(__linux__)
    #include <readline/history.h>
    #include <readline/readline.h>
#else
    #include <string.h>

    static char buffer[2048];

    char *readline(char *prompt)
    {
        fputs(prompt, stdout);
        fgets(buffer, 2048, stdin);
        size_t length = strlen(buffer);
        char *copy = (char *)malloc(length + 1);
        strcpy(copy, buffer);
        copy[length + 1] = '\0';
        return copy;
    }

    void add_history(char *unused) {}
#endif

static void repl(LoxVM *vm)
{
    for (;;)
    {
        char *source = readline(">>> ");
        if (source == NULL)
        {
            printf("\n");
            return;
        }

        add_history(source);
        interpret(vm, source);
        free(source);
    }
}

static char *readSource(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        printf("Could not open file '%s'. Did you spell it right?", path);
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(fileSize + 1);
    if (!buffer) return NULL;

    size_t length = fread(buffer, sizeof(char), fileSize, file);
    if (length < fileSize)
    {
        printf("Failed to read file '%s'", path);
        return NULL;
    }

    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

static void runSource(LoxVM *vm, const char *path)
{
    char *source = readSource(path);
    if (!source)
    {
        setExit(65);
        return;
    }

    InterpretResult result = interpret(vm, source);
    free(source);

    if (result == RESULT_COMPILE_ERROR) setExit(65);
    if (result == RESULT_RUNTIME_ERROR) setExit(70);
}

int main(int argc, char **argv)
{
    LoxVM vm;
    initVM(&vm);

    switch (argc)
    {
        case 1:
            repl(&vm);
            break;
        case 2:
            runSource(&vm, argv[1]);
            break;
        default:
            printf("Usage: lox [script]\n");
            break;
    }

    freeVM(&vm);
    return getExit();
}
