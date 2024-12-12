#pragma once
#include "Windows.h"

#define PRINT_LAST_ERROR(msg) { \
    DWORD error_code = GetLastError(); \
    LPVOID msgBuffer; \
    DWORD dw = FormatMessage( \
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, \
        NULL, \
        error_code, \
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), \
        (LPTSTR)&msgBuffer, \
        0, \
        NULL \
    ); \
    if (dw != 0) { \
        printf(msg " Error code %lu: %s\n", error_code, (LPCTSTR)msgBuffer); \
        LocalFree(msgBuffer); \
    } else { \
        printf(msg " Error: %lu (Unable to retrieve error message)\n", error_code); \
    } \
}