#include <stdio.h>
#include <stdlib.h>

#include "common.h"

int __stdcall InitCommonControlsEx(const void *information) {
    TRACE("%p", information);
    return 1;
}