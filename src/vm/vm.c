#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "natives.h"
#include "vm.h"

static void resetStack(LoxVM *vm)
{
    vm->stackTop = vm->stack;
    vm->frameCount = 0;
}

static void runtimeError(LoxVM *vm, const char *fmt, ...)
{
    fprintf(stderr, "Runtime Error: ");

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = vm->frameCount - 1; i >= 0; i--)
    {
        CallFrame *frame = &vm->frames[i];
        ObjFn *function = frame->function;
        size_t instruction = frame->ip - frame->function->chunk.code.data - 1;
        int line = getLine(&frame->function->chunk, (int)instruction);

        fprintf(stderr, "[line %d] in ", line);
        if (function->name == NULL) fprintf(stderr, "main\n");
        else fprintf(stderr, "%s()\n", function->name->value);
    }

    resetStack(vm);
}

void initVM(LoxVM *vm)
{
    resetStack(vm);
    vm->objects = NULL;
    tableInit(&vm->strings);
    tableInit(&vm->globals);
    vm->compiler = NULL;
    defineNatives(vm);
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

static bool call(LoxVM *vm, ObjFn *function, int argc)
{
    if (argc != function->arity)
    {
        runtimeError(vm, "Expected %d arguments but got %d instead", function->arity, argc);
        return false;
    }

    if (vm->frameCount == FRAMES_MAX)
    {
        runtimeError(vm, "Stack overflow");
        return false;
    }

    CallFrame *frame = &vm->frames[vm->frameCount++];
    frame->function = function;
    frame->ip = function->chunk.code.data;
    frame->slots = vm->stackTop - argc - 1;
    return true;
}

static bool callValue(LoxVM *vm, Value callee, int argc)
{
    if (IS_OBJ(callee))
    {
        switch (OBJ_TYPE(callee))
        {
            case OBJ_FUNCTION:
                return call(vm, AS_FUNCTION(callee), argc);
            case OBJ_NATIVE:
            {
                ObjNative *native = AS_NATIVE(callee);
                NativeFn nativeFn = AS_NATIVE_FN(callee);

                if (argc != native->arity)
                {
                    runtimeError(vm, "Expected %d arguments but got %d instead", native->arity, argc);
                    return false;
                }

                Value result = nativeFn(argc, vm->stackTop - argc);
                vm->stackTop -= argc + 1;
                pushVM(vm, result);
                return true;
            } break;
            default:
                break; // Non-callable object type
        }
    }

    runtimeError(vm, "Can only call functions and classes");
    return false;
}

static InterpretResult runInterpreter(LoxVM *vm)
{
    CallFrame *frame = &vm->frames[vm->frameCount - 1];

    #define READ_BYTE()     (*frame->ip++)
    #define READ_SHORT()    (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
    #define READ_CONSTANT() (frame->function->chunk.constants.data[READ_BYTE()])
    #define READ_STRING()   AS_STRING(READ_CONSTANT())
    #define LOAD_FRAME()    (frame = &vm->frames[vm->frameCount - 1])
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
            disassembleInstruction(
                &frame->function->chunk,
                (int)(frame->ip - frame->function->chunk.code.data)
            );
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
            {
                Value result = popVM(vm);
                vm->frameCount--;

                if (vm->frameCount == 0)
                {
                    popVM(vm);
                    return RESULT_OK;
                }

                vm->stackTop = frame->slots;
                pushVM(vm, result);
                LOAD_FRAME();
            } break;
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
                pushVM(vm, frame->slots[slot]);
            } break;
            case OP_SET_LOCAL:
            {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peekVM(vm, 0);
            } break;
            case OP_CALL:
            {
                int argc = READ_BYTE();
                if (!callValue(vm, peekVM(vm, argc), argc))
                {
                    return RESULT_RUNTIME_ERROR;
                }
                LOAD_FRAME();
            } break;

            /* Jump Instructions */
            case OP_JUMP:
            {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
            } break;
            case OP_JUMP_IF:
            {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peekVM(vm, 0))) frame->ip += offset;
            } break;
            case OP_LOOP:
            {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
            } break;
            case OP_AND:
            {
                uint16_t offset = READ_SHORT();
                Value condition = peekVM(vm, 0);

                if (isFalsey(condition)) frame->ip += offset;
                else popVM(vm);
            } break;
            case OP_OR:
            {
                uint16_t offset = READ_SHORT();
                Value condition = peekVM(vm, 0);

                if (isFalsey(condition)) popVM(vm);
                else frame->ip += offset;
            } break;
        }
    }

    #undef BINARY_OP
    #undef LOAD_FRAME
    #undef READ_STRING
    #undef READ_CONSTANT
    #undef READ_SHORT
    #undef READ_BYTE
}

InterpretResult interpret(LoxVM *vm, char *source)
{
    ObjFn *function = compile(vm, source);
    if (function == NULL) return RESULT_COMPILE_ERROR;

    pushVM(vm, OBJ_VAL(function));
    call(vm, function, 0);

    return runInterpreter(vm);
}
