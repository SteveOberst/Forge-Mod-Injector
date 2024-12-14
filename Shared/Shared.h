// Shared.h: Headerdatei für Ihr Ziel.

#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <Windows.h>
#include "PayloadParameters.h"

#define MOD_FILE_LOC "Global\\FMISharedModFile"
#define MAIN_CLASS_LOC "Global\\FMISharedMainClass"


// Ensure that the memory layout is consistent
#pragma pack(push, 1)
typedef struct  
{
	uint64_t bufSize;
	char fileBuf[1];
} SHARED_FILE, *LPSHARED_FILE;
#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

	// Function declarations for C
	bool PublishFile(const char* buf, size_t bufSize, LPCSTR loc);

	bool GetFile(HANDLE* hMapFileOut, LPCSTR loc, char** bufOut, DWORD* bufLenOut);

#ifdef __cplusplus
}
#endif

