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
    tableInit(&vm->strings);
    tableInit(&vm->globals);
    vm->compiler = NULL;
}

void freeVM(LoxVM *vm)
{
    tableFree(&vm->globals);
    tableFree(&vm->strings);
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
    #define READ_SHORT()    (vm->ip += 2, (uint16_t)((vm->ip[-2] << 8) | vm->ip[-1]))
    #define READ_CONSTANT() (vm->chunk->constants.data[READ_BYTE()])
    #define READ_STRING()   AS_STRING(READ_CONSTANT())
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
            /* Simple Instructions */
            case OP_NIL:
                pushVM(vm, NIL_VAL);
                break;
            case OP_FALSE:
                pushVM(vm, BOOL_VAL(false));
                break;
            case OP_TRUE:
                pushVM(vm, BOOL_VAL(true));
                break;
            case OP_POP:
                popVM(vm);
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
                return RESULT_OK;
            case OP_PRINT:
            {
                printValue(popVM(vm));
                printf("\n");
            } break;

            /* Constant Instructions */
            case OP_CONSTANT:
            {
                Value constant = READ_CONSTANT();
                pushVM(vm, constant);
            } break;
            case OP_DEFINE_GLOBAL:
            {
                ObjString *name = READ_STRING();
                tableSet(&vm->globals, name, peekVM(vm, 0));
                popVM(vm);
            } break;
            case OP_GET_GLOBAL:
            {
                ObjString *name = READ_STRING();
                Value value;

                if (!tableGet(&vm->globals, name, &value))
                {
                    runtimeError(vm, "Undefined variable '%s'", name->value);
                    return RESULT_RUNTIME_ERROR;
                }

                pushVM(vm, value);
            } break;
            case OP_SET_GLOBAL:
            {
                ObjString *name = READ_STRING();
                if (tableSet(&vm->globals, name, peekVM(vm, 0)))
                {
                    tableDelete(&vm->globals, name);
                    runtimeError(vm, "Undefined variable '%s'", name->value);
                    return RESULT_RUNTIME_ERROR;
                }
            } break;

            /* Byte Instructions */
            case OP_GET_LOCAL:
            {
                uint8_t slot = READ_BYTE();
                pushVM(vm, vm->stack[slot]);
            } break;
            case OP_SET_LOCAL:
            {
                uint8_t slot = READ_BYTE();
                vm->stack[slot] = peekVM(vm, 0);
            } break;

            /* Jump Instructions */
            case OP_JUMP:
            {
                uint16_t offset = READ_SHORT();
                vm->ip += offset;
            } break;
            case OP_JUMP_IF:
            {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peekVM(vm, 0))) vm->ip += offset;
            } break;
            case OP_LOOP:
                break;
            case OP_AND:
            {
                uint16_t offset = READ_SHORT();
                Value condition = peekVM(vm, 0);

                if (isFalsey(condition)) vm->ip += offset;
                else popVM(vm);
            } break;
            case OP_OR:
            {
                uint16_t offset = READ_SHORT();
                Value condition = peekVM(vm, 0);

                if (isFalsey(condition)) popVM(vm);
                else vm->ip += offset;
            } break;
        }
    }

    #undef BINARY_OP
    #undef READ_STRING
    #undef READ_CONSTANT
    #undef READ_SHORT
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
