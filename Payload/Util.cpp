#include "Util.h"

#include <Windows.h>
#include <stdio.h>

const char* getTempFilePath(const char* fileName) {
	static char path[MAX_PATH];
	DWORD tempPathLen = GetTempPathA(MAX_PATH, path);
	if (tempPathLen == 0 || tempPathLen > MAX_PATH) {
		return NULL;
	}
	snprintf(path + tempPathLen, MAX_PATH - tempPathLen, "%s", fileName);
	return path;
}

void AllocateDebugConsole()
{
	AllocConsole();
	FILE* file;
	freopen_s(&file, "CONOUT$", "w", stdout);
	freopen_s(&file, "CONOUT$", "w", stderr);
}