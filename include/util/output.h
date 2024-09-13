#pragma once
#include "base.h"

#include <stdio.h>

// puts without newline
static inline int
put(_In_z_ const char* str)
{
    return fputs(str, stdout);
}