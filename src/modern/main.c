#include <stdio.h>

#include "lox_array.h"
#include "lox_vm.h"

//--------------------------------------------------------------------------------------------------

lox_define_array_t(uint8_t, Byte);

static LoxVM vm = { 0 };

//--------------------------------------------------------------------------------------------------

int main(void)
{
    ByteArray code;
    lox_init_array(&code);

    lox_array_append(&vm, &code, 34);
    lox_array_append(&vm, &code, 35);

    printf("%d + %d = %d\n", code.data[0], code.data[1], code.data[0] + code.data[1]);

    lox_clear_array(&vm, &code);
    return 0;
}
