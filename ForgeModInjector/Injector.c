#include "Injector.h"
#include <TlHelp32.h>

DWORD_PTR GetModuleBaseAddress(DWORD procId, const char* moduleName) {

	if (!moduleName)
	{
		printf("Module name is null.\n");
		return 0;
	}

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap == INVALID_HANDLE_VALUE) {
		return 0;
	}

	MODULEENTRY32 me32;
	me32.dwSize = sizeof(MODULEENTRY32);

	if (Module32First(hSnap, &me32)) {
		do {
			printf("Module found: %s\n", me32.szModule);
			if (_stricmp(me32.szModule, moduleName) == 0) {
				CloseHandle(hSnap);
				return (DWORD_PTR)me32.modBaseAddr;
			}
		} while (Module32Next(hSnap, &me32));
	}

	CloseHandle(hSnap);
	return 0;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	char window_title[MAX_PATH];

	INJECT_PARAMS* params = (INJECT_PARAMS*)lParam;
	if (!params->target_window)
	{
		printf("No target window provided.\n");
		return FALSE;
	}

	const char* target = params->target_window;
	

	GetWindowTextA(hwnd, window_title, MAX_PATH);
	if (strcmp(window_title, target) == 0)
	{
		DWORD proc_id;
		GetWindowThreadProcessId(hwnd, &proc_id);

		params->dwProcId = proc_id;
		return FALSE;
	}

	return TRUE;
}

void find_process_id(INJECT_PARAMS* params)
{
	EnumWindows(&EnumWindowsProc, (LPARAM)params);
}

DWORD WINAPI Inject(INJECT_PARAMS* params)
{
	if (!params->dwProcId)
	{
		find_process_id(params);
		if (!params->dwProcId)
		{
			PRINT_LAST_ERROR("Failed to find process id.");
			printf("could not find process id for window: %s, executable target: %s", params->target_window, params->target_process_executable);
			return 0;
		}
	}

	HMODULE hModKernel32 = GetModuleHandleA("kernel32.dll");
	if (!hModKernel32)
	{
		PRINT_LAST_ERROR("Failed to get handle to kernel32.dll.");
		return 0;
	}

	void* load_library_addr = GetProcAddress(hModKernel32, "LoadLibraryA");
	if (!load_library_addr)
	{
		PRINT_LAST_ERROR("Failed to get address of LoadLibraryA.");
		return 0;
	}

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, params->dwProcId);
	if (!hProc)
	{
		PRINT_LAST_ERROR("Failed to open process.");
		return 0;
	}

	if (!params->payload_file_path)
	{
		PRINT_LAST_ERROR("No file path for the payload provided.");
		return 0;
	}

	FILE* f = fopen(params->payload_file_path, "r");
	if (!f)
	{
		printf("Unable to open payload file %s. Make sure it exists at the given path.\n", params->payload_file_path);
		return 0;
	}

	fclose(f);

	LPVOID lpRemoteMem = VirtualAllocEx(
		hProc,
		NULL,
		strlen(params->payload_file_path) + 1,
		MEM_RESERVE | MEM_COMMIT,
		PAGE_READWRITE
	);

	if (!lpRemoteMem)
	{
		PRINT_LAST_ERROR("Failed to allocate memory in target process.");
		CloseHandle(hProc);
		return 0;
	}

	BOOL bWriteRes = WriteProcessMemory(
		hProc,
		lpRemoteMem,
		params->payload_file_path,
		strlen(params->payload_file_path) + 1,
		NULL
	);

	if (!bWriteRes)
	{
		PRINT_LAST_ERROR("Failed to write memory in target process.");
		CloseHandle(hProc);
		return 0;
	}

	HANDLE hThread = CreateRemoteThread(
		hProc,
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)load_library_addr,
		lpRemoteMem,
		0,
		NULL
	);

	if (!hThread)
	{
		PRINT_LAST_ERROR("Failed to create remote thread.");
		CloseHandle(hProc);
		return 0;
	}

	WaitForSingleObject(hThread, INFINITE);

	if (!VirtualFreeEx(hProc, lpRemoteMem, 0, MEM_RELEASE))
	{
		PRINT_LAST_ERROR("Failed to free memory in target process.");
		CloseHandle(hThread);
		CloseHandle(hProc);
		return 0;
	}

	CloseHandle(hProc);
	CloseHandle(hThread);
	return TRUE;
}
