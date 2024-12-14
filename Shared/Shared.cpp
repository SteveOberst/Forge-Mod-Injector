#include "Shared.h"

/// <summary>
/// Note that the buffer is copied into the shared memory. Once no handle is kept to the shared memory
/// the shared memory is released by the OS. So make sure to keep the handle until it's not needed anymore.
/// </summary>
/// <param name="buf">the buf to read the file from</param> 
/// <param name="bufSize">the size of the buffer</param> 
/// <param name="lpMapFile">the handle to the shared memory</param> 
/// <returns></returns>
bool PublishFile(const char* buf, size_t bufSize, LPHANDLE lpMapFile, LPCWSTR loc)
{
	// Please end my suffering
	const size_t totalSize = sizeof(SHARED_FILE) + bufSize - 1;

	// As the target process in running with lower priviliges we need to neglect 
	// proper security measures and as we're being lazy we allow everyone the shared mem
	SECURITY_DESCRIPTOR sd;
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE); // Allow access to everyone

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = &sd; // Use the custom security descriptor
	sa.bInheritHandle = FALSE;

	HANDLE hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // Use paging file
		&sa,                     // Default security
		PAGE_READWRITE,          // Read/write access
		0,                       // Maximum object size (high-order DWORD)
		totalSize,               // Maximum object size (low-order DWORD)
		loc);				     // Name of mapping object

	if (hMapFile == NULL) {
		printf("Could not create file mapping object\n");
		return 1;
	}

	LPVOID pBuf = MapViewOfFile(
		hMapFile,               // Handle to map object
		FILE_MAP_ALL_ACCESS,    // Read/write permission
		0,
		0,
		totalSize);

	if (pBuf == NULL) {
		printf("Could not map view of file\n");
		CloseHandle(hMapFile);
		return 1;
	}

	LPSHARED_FILE sharedFile = (LPSHARED_FILE)pBuf;
	sharedFile->bufSize = bufSize;
	memcpy(sharedFile->fileBuf, buf, bufSize);

	UnmapViewOfFile(pBuf);

	return true;
}

bool GetFile(HANDLE* hMapFileOut, LPCWSTR loc, char** bufOut, DWORD* bufLenOut) {
	HANDLE hMapFile = OpenFileMapping(
		FILE_MAP_READ,          // Read access
		FALSE,                  // Do not inherit the name
		loc);				    // Name of mapping object

	if (hMapFile == NULL) {
		printf("Error in OpenFileMapping(): (%lu)\n", GetLastError());
		POPUPA("Could not open file mapping object");
		return false;
	}

	LPVOID pBuf = MapViewOfFile(
		hMapFile,               // Handle to map object
		FILE_MAP_ALL_ACCESS,    // Read/write permission
		0,
		0,
		0);

	if (pBuf == NULL) {
		POPUPA("Could not map view of file");
		printf("Error in MapViewOfFile(): (%lu)\n", GetLastError());
		CloseHandle(hMapFile);
		return false;
	}

	LPSHARED_FILE sharedFile = (LPSHARED_FILE)pBuf;
	*bufLenOut = sharedFile->bufSize;

	char* buffer = (char*)malloc(sharedFile->bufSize);
	if (buffer == NULL) {
		POPUPA("Memory allocation failed");
		UnmapViewOfFile(pBuf);
		CloseHandle(hMapFile);
		return false;
	}

	memcpy(buffer, sharedFile->fileBuf, sharedFile->bufSize);
	*bufOut = buffer;
	*hMapFileOut = hMapFile;

	CloseHandle(hMapFile);
	return true;
}

