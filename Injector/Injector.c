#include "Injector.h"


#include <TlHelp32.h>
#include <stdbool.h>

#include "Shared.h"
#include "Filesystem.h"

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

	LPINJECT_PARAMS params = (LPINJECT_PARAMS)lParam;
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

void find_process_id(LPINJECT_PARAMS params)
{
	EnumWindows(&EnumWindowsProc, (LPARAM)params);
}

DWORD WINAPI Inject(LPINJECT_PARAMS params, LPPAYLOAD_PARAMS payload_params)
{
	if (!params->dwProcId)
	{
		find_process_id(params);
		if (!params->dwProcId)
		{
			PRINT_LAST_ERROR("Failed to find process id.");
			printf("could not find process id for window: %s", params->target_window);
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
		CloseHandle(hProc);
		return 0;
	}

	if (!check_file_exists(params->payload_file_path))
	{
		printf("Unable to open payload file %s. Make sure it exists at the given path.\n", params->payload_file_path);
		CloseHandle(hProc);
		return 0;
	}

	if (!check_file_exists(params->mod_file))
	{
		printf("Unable to open mod file %s. Make sure it exists at the given path.\n", params->mod_file);
		CloseHandle(hProc);
		return 0;
	}

	size_t buf_len;
	char* buf = read_file(params->mod_file, &buf_len);
	if (!buf)
	{
		PRINT_LAST_ERROR("Failed to read mod file.");
		CloseHandle(hProc);
		free(buf);
		return 0;
	}

	if (!PublishFile(buf, buf_len, MOD_FILE_LOC))
	{
		PRINT_LAST_ERROR("Failed to publish mod file.");
		CloseHandle(hProc);
		free(buf);
		return 0;
	}

	if (!PublishFile(payload_params->main_class, strlen(payload_params->main_class) + 1, MAIN_CLASS_LOC))
	{
		PRINT_LAST_ERROR("Failed to publish payload parameters.");
		CloseHandle(hProc);
		free(buf);
		return 0;
	}

	// Write the payload path to the target memory
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
		free(buf);
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
		free(buf);
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
		free(buf);
		return 0;
	}

	WaitForSingleObject(hThread, INFINITE);

	if (!VirtualFreeEx(hProc, lpRemoteMem, 0, MEM_RELEASE))
	{
		PRINT_LAST_ERROR("Failed to free memory in target process.");
		CloseHandle(hProc);
		CloseHandle(hThread);
		free(buf);
 		return 0;
	}

	CloseHandle(hProc);
	CloseHandle(hThread);
	return TRUE;
}
