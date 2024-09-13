#include "util/misc.h"

#ifdef _WIN32
#include <windows.h>

void
us_sleep(unsigned long ms)
{
    Sleep(ms);
}
#else
#include <time.h>

void
us_sleep(unsigned long ms)
{
    nanosleep(
        (const struct timespec[]){
            {0, ms * 1000000L}
    },
        NULL
    );
}
#endif
