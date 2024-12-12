#pragma once
#include <stdio.h>
#include <Windows.h>

#ifdef _WIN32
#ifdef PAYLOAD_EXPORTS
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif
#else
#define DLLEXPORT __attribute__((visibility("default")))
#endif

#define PRINTN(fmt, ...) printf(fmt"\n", ##__VA_ARGS__)