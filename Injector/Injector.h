#pragma once

#include <stddef.h>
#include <stdio.h>
#include <Windows.h>
#include "WinAPIDebugPrint.h"
#include "PayloadParameters.h"

typedef struct {
	const char* payload_file_path;
	const char* payload_module_name;
	const char* target_window;
	const char* mod_file;
	DWORD dwProcId;
} INJECT_PARAMS, * LPINJECT_PARAMS;

#ifdef __cplusplus
extern "C"
{
#endif

	DWORD WINAPI Inject(LPINJECT_PARAMS params, LPPAYLOAD_PARAMS payload_params);

#ifdef __cplusplus
}
#endif