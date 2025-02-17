#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "vm.h"

static void resetStack(LoxVM *vm)
{
    vm->stackTop = vm->stack;
}

static void runtimeError(LoxVM *vm, const char *fmt, ...)
{
    fprintf(stderr, "Runtime Error: ");

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputs("\n", stderr);

    int instruction = (int)(vm->ip - vm->chunk->code.data - 1);
    int line = getLine(vm->chunk, instruction);
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack(vm);
}

void initVM(LoxVM *vm)
{
    resetStack(vm);
    vm->objects = NULL;
}

void freeVM(LoxVM *vm)
{
    freeObjects(vm);
}

void pushVM(LoxVM *vm, Value value)
{
    *vm->stackTop++ = value;
}

Value popVM(LoxVM *vm)
{
    return *(--vm->stackTop);
}

Value peekVM(LoxVM *vm, int distance)
{
    return *(vm->stackTop - 1 - distance);
}

static void concatenate(LoxVM *vm)
{
    ObjString *b = AS_STRING(popVM(vm));
    ObjString *a = AS_STRING(popVM(vm));

    uint32_t length = a->length + b->length;
    char *chars = ALLOCATE_ARRAY(char, length + 1);
    memcpy(chars, a->value, a->length);
    memcpy(chars + a->length, b->value, b->length);
    chars[length] = '\0';

    ObjString *result = newString(vm, chars);
    pushVM(vm, OBJ_VAL(result));
}

static InterpretResult runInterpreter(LoxVM *vm)
{
    #define READ_BYTE()     (*vm->ip++)
    #define READ_CONSTANT() (vm->chunk->constants.data[READ_BYTE()])
    #define BINARY_OP(valueType, op)                                                \
        do                                                                          \
        {                                                                           \
            if (!IS_NUMBER(peekVM(vm, 0)) && !IS_NUMBER(peekVM(vm, 1)))             \
            {                                                                       \
                runtimeError(vm, "Binary (non-addition) operands must be numbers"); \
                return RESULT_RUNTIME_ERROR;                                        \
            }                                                                       \
            double b = AS_NUMBER(popVM(vm));                                        \
            double a = AS_NUMBER(popVM(vm));                                        \
            pushVM(vm, valueType(a op b));                                          \
        } while (false)

    for (;;)
    {
        #if TRACE_INSTRUCTIONS
            printf("        ");
            for (Value *slot = vm->stack; slot < vm->stackTop; slot++)
            {
                printf("[ ");
                printValue(*slot);
                printf(" ]");
            }
            printf("\n");
            disassembleInstruction(vm->chunk, (int)(vm->ip - vm->chunk->code.data));
        #endif

        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
            case OP_NIL:
                pushVM(vm, NIL_VAL);
                break;
            case OP_FALSE:
                pushVM(vm, BOOL_VAL(false));
                break;
            case OP_TRUE:
                pushVM(vm, BOOL_VAL(true));
                break;
            case OP_NEGATE:
            {
                if (!IS_NUMBER(peekVM(vm, 0)))
                {
                    runtimeError(vm, "Negation operand must be a number");
                    return RESULT_RUNTIME_ERROR;
                }
                pushVM(vm, NUMBER_VAL(-AS_NUMBER(popVM(vm))));
            } break;
            case OP_NOT:
                pushVM(vm, BOOL_VAL(isFalsey(popVM(vm))));
                break;
            case OP_ADD:
            {
                if (IS_STRING(peekVM(vm, 0)) && IS_STRING(peekVM(vm, 1)))
                {
                    concatenate(vm);
                }
                else if (IS_NUMBER(peekVM(vm, 0)) && IS_NUMBER(peekVM(vm, 1)))
                {
                    double b = AS_NUMBER(popVM(vm));
                    double a = AS_NUMBER(popVM(vm));
                    pushVM(vm, NUMBER_VAL(a + b));
                }
                else
                {
                    runtimeError(vm, "Binary (addition) operands must be numbers or strings (but not both)");
                    return RESULT_RUNTIME_ERROR;
                }
            } break;
            case OP_SUBTRACT:
                BINARY_OP(NUMBER_VAL, -);
                break;
            case OP_MULTIPLY:
                BINARY_OP(NUMBER_VAL, *);
                break;
            case OP_DIVIDE:
                BINARY_OP(NUMBER_VAL, /);
                break;
            case OP_EQUAL:
            {
                Value b = popVM(vm);
                Value a = popVM(vm);
                pushVM(vm, BOOL_VAL(valuesEqual(a, b)));
            } break;
            case OP_GREATER:
                BINARY_OP(BOOL_VAL, >);
                break;
            case OP_LESS:
                BINARY_OP(BOOL_VAL, <);
                break;
            case OP_RETURN:
            {
                printValue(popVM(vm));
                printf("\n");
                return RESULT_OK;
            }
            case OP_CONSTANT:
            {
                Value constant = READ_CONSTANT();
                pushVM(vm, constant);
            } break;
        }
    }

    #undef BINARY_OP
    #undef READ_CONSTANT
    #undef READ_BYTE
}

InterpretResult interpret(LoxVM *vm, char *source)
{
    Chunk chunk;
    initChunk(&chunk);

    if (!compile(vm, &chunk, source))
    {
        freeChunk(&chunk);
        return RESULT_COMPILE_ERROR;
    }

    vm->chunk = &chunk;
    vm->ip = vm->chunk->code.data;

    InterpretResult result = runInterpreter(vm);
    freeChunk(&chunk);
    return result;
}
