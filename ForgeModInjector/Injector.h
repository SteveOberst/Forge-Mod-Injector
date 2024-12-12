#pragma once

#include <stddef.h>
#include <stdio.h>
#include <Windows.h>
#include "WinAPIDebugPrint.h"
#include "WinAPIDefs.h"

typedef struct {
	const char* payload_file_path;
	const char* payload_module_name;
	const char* target_window;
	const char* target_process_executable; 
	DWORD dwProcId;
} INJECT_PARAMS;

DWORD WINAPI Inject(INJECT_PARAMS* params);