#include "debug.h"
#include "opcode.h"
#include "value.h"

//------------------------------------------------------------------------------

int main(void)
{
    LoxVM *vm = newVM();
    interpret(vm, NULL);
    freeVM(vm);
    return 0;
}
