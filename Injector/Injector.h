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
} INJECT_PARAMS, *LPINJECT_PARAMS;

DWORD WINAPI Inject(LPINJECT_PARAMS params, LPPAYLOAD_PARAMS payload_params);