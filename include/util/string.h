#pragma once
#include "base.h"

int strcatc(_Inout_z_ char* str, char c);
bool strislwr(_In_z_ const char* str);
bool iswordend(const char c);

#ifdef _WIN32
_Ret_z_ char* stpcpy(_Inout_z_ char* dest, _In_z_ const char* src);
#endif