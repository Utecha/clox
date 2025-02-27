#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "natives.h"
#include "vm.h"

static void defineNative(LoxVM *vm, const char *name, NativeFn function, int arity)
{
    pushVM(vm, OBJ_VAL(newStringLength(vm, name, strlen(name))));
    pushVM(vm, OBJ_VAL(newNative(vm, function, arity)));

    tableSet(&vm->globals, AS_STRING(vm->stack[0]), vm->stack[1]);

    popVM(vm);
    popVM(vm);
}

static Value clockNative(int argc, Value *args)
{
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value absNative(int argc, Value *args)
{
    return NUMBER_VAL(fabs(AS_NUMBER(args[0])));
}

static Value acosNative(int argc, Value *args)
{
    return NUMBER_VAL(acos(AS_NUMBER(args[0])));
}

static Value asinNative(int argc, Value *args)
{
    return NUMBER_VAL(asin(AS_NUMBER(args[0])));
}

static Value atanNative(int argc, Value *args)
{
    return NUMBER_VAL(atan(AS_NUMBER(args[0])));
}

static Value atan2Native(int argc, Value *args)
{
    return NUMBER_VAL(atan2(AS_NUMBER(args[0]), AS_NUMBER(args[1])));
}

static Value cbrtNative(int argc, Value *args)
{
    return NUMBER_VAL(cbrt(AS_NUMBER(args[0])));
}

static Value ceilNative(int argc, Value *args)
{
    return NUMBER_VAL(ceil(AS_NUMBER(args[0])));
}

static Value cosNative(int argc, Value *args)
{
    return NUMBER_VAL(cos(AS_NUMBER(args[0])));
}

static Value floorNative(int argc, Value *args)
{
    return NUMBER_VAL(floor(AS_NUMBER(args[0])));
}

static Value fractionNative(int argc, Value *args)
{
    double unused;
    return NUMBER_VAL(modf(AS_NUMBER(args[0]), &unused));
}

static Value isIntNative(int argc, Value *args)
{
    double value = AS_NUMBER(args[0]);
    if (isnan(value) || isinf(value)) return BOOL_VAL(false);
    return BOOL_VAL(trunc(value) == value);
}

static Value truncNative(int argc, Value *args)
{
    double integer;
    modf(AS_NUMBER(args[0]), &integer);
    return NUMBER_VAL(integer);
}

static Value logNative(int argc, Value *args)
{
    return NUMBER_VAL(log(AS_NUMBER(args[0])));
}

static Value log2Native(int argc, Value *args)
{
    return NUMBER_VAL(log2(AS_NUMBER(args[0])));
}

static Value minNative(int argc, Value *args)
{
    double value = AS_NUMBER(args[0]);
    double other = AS_NUMBER(args[1]);
    return NUMBER_VAL(value <= other ? value : other);
}

static Value maxNative(int argc, Value *args)
{
    double value = AS_NUMBER(args[0]);
    double other = AS_NUMBER(args[1]);
    return NUMBER_VAL(value > other ? value : other);
}

static Value roundNative(int argc, Value *args)
{
    return NUMBER_VAL(round(AS_NUMBER(args[0])));
}

static Value sinNative(int argc, Value *args)
{
    return NUMBER_VAL(sin(AS_NUMBER(args[0])));
}

static Value sqrtNative(int argc, Value *args)
{
    return NUMBER_VAL(sqrt(AS_NUMBER(args[0])));
}

static Value tanNative(int argc, Value *args)
{
    return NUMBER_VAL(tan(AS_NUMBER(args[0])));
}

void defineNatives(LoxVM *vm)
{
    defineNative(vm, "clock", clockNative, 0);
    defineNative(vm, "abs", absNative, 1);
    defineNative(vm, "acos", acosNative, 1);
    defineNative(vm, "asin", asinNative, 1);
    defineNative(vm, "atan", atanNative, 1);
    defineNative(vm, "atan2", atan2Native, 2);
    defineNative(vm, "cbrt", cbrtNative, 1);
    defineNative(vm, "ceil", ceilNative, 1);
    defineNative(vm, "cos", cosNative, 1);
    defineNative(vm, "floor", floorNative, 1);
    defineNative(vm, "fraction", fractionNative, 1);
    defineNative(vm, "isInt", isIntNative, 1);
    defineNative(vm, "trunc", truncNative, 1);
    defineNative(vm, "log", logNative, 1);
    defineNative(vm, "log2", log2Native, 1);
    defineNative(vm, "min", minNative, 2);
    defineNative(vm, "max", maxNative, 2);
    defineNative(vm, "round", roundNative, 1);
    defineNative(vm, "sin", sinNative, 1);
    defineNative(vm, "sqrt", sqrtNative, 1);
    defineNative(vm, "tan", tanNative, 1);
}
