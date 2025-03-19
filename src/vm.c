#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "compiler.h"
#include "memory.h"
#include "value.h"
#include "vm.h"

#ifdef DEBUG
#include "debug.h"
#endif

static void reset_stack(LoxVM *vm)
{
    vm->stackTop = vm->stack;
}

void init_lox_vm(LoxVM *vm)
{
    reset_stack(vm);
    vm->objects = NULL;
    vm->compiler = NULL;
}

void free_lox_vm(LoxVM *vm)
{
    free_objects(vm);
}

void push_stack(LoxVM *vm, Value value)
{
    *vm->stackTop++ = value;
}

Value pop_stack(LoxVM *vm)
{
    return *(--vm->stackTop);
}

static Value peek_stack(LoxVM *vm, int distance)
{
    return vm->stackTop[-1 - distance];
}

static void runtime_error(LoxVM *vm, const char *fmt, ...)
{
    fprintf(stderr, "Runtime Error: ");

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");

    size_t instruction = vm->ip - vm->chunk->code - 1;
    int line = vm->chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    reset_stack(vm);
}

static bool is_falsey(Value value)
{
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate(LoxVM *vm)
{
    ObjString *b = AS_STRING(pop_stack(vm));
    ObjString *a = AS_STRING(pop_stack(vm));

    int length = a->length + b->length;
    char *data = ALLOCATE_ARRAY(vm, char, length + 1);
    memcpy(data, a->data, a->length);
    memcpy(data + a->length, b->data, b->length);
    data[length] = '\0';

    ObjString *result = take_string(vm, data, length);
    push_stack(vm, OBJ_VAL(result));
}

static InterpretResult run_lox_vm(LoxVM *vm)
{
    #define READ_BYTE()         (*vm->ip++)
    #define READ_CONSTANT()     (vm->chunk->constants.values[READ_BYTE()])
    #define BINARY_OP(op, valueType)                                            \
        do                                                                      \
        {                                                                       \
            if (!IS_NUMBER(peek_stack(vm, 0)) || !IS_NUMBER(peek_stack(vm, 1))) \
            {                                                                   \
                runtime_error(vm, "Binary operands must be numbers");           \
                return RESULT_RUNTIME_ERROR;                                    \
            }                                                                   \
            double b = AS_NUMBER(pop_stack(vm));                                \
            double a = AS_NUMBER(pop_stack(vm));                                \
            push_stack(vm, valueType(a op b));                                  \
        } while (false)

    for (;;)
    {
        #if DEBUG_TRACE_INSTRUCTIONS
            printf("        ");
            for (Value *slot = vm->stack; slot < vm->stackTop; slot++)
            {
                printf("[ ");
                print_value(*slot);
                printf(" ]");
            }
            printf("\n");
            disassemble_instruction(vm->chunk, (int)(vm->ip - vm->chunk->code));
        #endif

        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
            case OP_NIL:
            {
                push_stack(vm, NIL_VAL);
            } break;
            case OP_FALSE:
            {
                push_stack(vm, BOOL_VAL(false));
            } break;
            case OP_TRUE:
            {
                push_stack(vm, BOOL_VAL(true));
            } break;
            case OP_NOT:
            {
                push_stack(vm, BOOL_VAL(is_falsey(pop_stack(vm))));
            } break;
            case OP_NEGATE:
            {
                if (!IS_NUMBER(peek_stack(vm, 0)))
                {
                    runtime_error(vm, "Unary negation operand must be an number");
                    return RESULT_RUNTIME_ERROR;
                }

                push_stack(vm, NUMBER_VAL(-AS_NUMBER(pop_stack(vm))));
            } break;
            case OP_ADD:
            {
                if (IS_STRING(peek_stack(vm, 0)) && IS_STRING(peek_stack(vm, 1)))
                {
                    concatenate(vm);
                }
                else if (IS_NUMBER(peek_stack(vm, 0)) && IS_NUMBER(peek_stack(vm, 1)))
                {
                    double b = AS_NUMBER(pop_stack(vm));
                    double a = AS_NUMBER(pop_stack(vm));
                    push_stack(vm, NUMBER_VAL(a + b));
                }
                else
                {
                    runtime_error(vm, "Binary (addition) operands must be numbers or strings (but not both)");
                    return RESULT_RUNTIME_ERROR;
                }
            } break;
            case OP_SUBTRACT:
            {
                BINARY_OP(-, NUMBER_VAL);
            } break;
            case OP_MULTIPLY:
            {
                BINARY_OP(*, NUMBER_VAL);
            } break;
            case OP_DIVIDE:
            {
                BINARY_OP(/, NUMBER_VAL);
            } break;
            case OP_EQUAL:
            {
                Value b = pop_stack(vm);
                Value a = pop_stack(vm);
                push_stack(vm, BOOL_VAL(values_equal(a, b)));
            } break;
            case OP_GREATER:
            {
                BINARY_OP(>, BOOL_VAL);
            } break;
            case OP_LESS:
            {
                BINARY_OP(<, BOOL_VAL);
            } break;
            case OP_RETURN:
            {
                print_value(pop_stack(vm));
                printf("\n");
                return RESULT_OK;
            }
            case OP_CONSTANT:
            {
                Value constant = READ_CONSTANT();
                push_stack(vm, constant);
            } break;
        }
    }

    #undef BINARY_OP
    #undef READ_CONSTANT
    #undef READ_BYTE
}

InterpretResult interpret_lox_vm(LoxVM *vm, const char *source)
{
    #if DEBUG_DUMP_TOKENS
        dump_tokens(source);
        return RESULT_OK;
    #else
        Chunk chunk;
        init_chunk(&chunk);

        if(!compile(vm, source, &chunk))
        {
            free_chunk(vm, &chunk);
            return RESULT_COMPILE_ERROR;
        }

        vm->chunk = &chunk;
        vm->ip = vm->chunk->code;
        InterpretResult result = run_lox_vm(vm);

        free_chunk(vm, &chunk);
        return result;
    #endif
}
