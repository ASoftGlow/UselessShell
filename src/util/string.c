#include "util/string.h"
#include <ctype.h>
#include <stddef.h>
#include <string.h>

bool
strislwr(_In_z_ const char* str)
{
    int c = 0;
    while (str[c])
    {
        if (isalpha(str[c]) && !islower(str[c])) return false;
        c++;
    }
    return true;
}

static char word_delimiters[] = " /\\()\"'-.,:;<>~!@#$%^&*|+=[]{}?";

bool
iswordend(const char c)
{
    return strchr(word_delimiters, c);
}

int
strcatc(_Inout_z_ char* str, char c)
{
    size_t len = strlen(str);
    str[len] = c;
    str[len + 1] = 0;
    return 0;
}

#ifdef _WIN32
_Ret_z_ char*
stpcpy(_Inout_z_ char* dest, _In_z_ const char* src)
{
    const size_t len = strlen(src);
    return (char*)memcpy(dest, src, len + 1) + len;
}
#endif