#include <stdio.h>

#include "lox_array.h"

//--------------------------------------------------------------------------------------------------

lox_define_array_t(uint8_t, Byte);

static LoxVM *vm = nullptr;

//--------------------------------------------------------------------------------------------------

int main(void)
{
    ByteArray code;

    vm = lox_new_vm(nullptr);
    lox_init_array(&code);

    lox_array_append(vm, &code, 34);
    lox_array_append(vm, &code, 35);

    printf("%d + %d = %d\n", code.data[0], code.data[1], code.data[0] + code.data[1]);

    lox_clear_array(vm, &code);
    lox_free_vm(vm);

    return 0;
}
