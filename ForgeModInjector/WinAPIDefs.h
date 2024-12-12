#pragma once
#include <Windows.h>

#pragma pack(push, 1)
typedef struct {
	HMODULE hRemoteModule;
	LPCSTR lpProcName;
} GET_PROC_ADDRESS_PARAMS;
#pragma pack(pop)