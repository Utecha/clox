#include "internal.h"

static int exitcode = 0;

int getExit(void)
{
    return exitcode;
}

void setExit(int code)
{
    exitcode = code;
}
