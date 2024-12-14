#include "Shared.h"
#include "WinAPIDebugPrint.h"

/// <summary>
/// Note that the buffer is copied into the shared memory. Once no handle is kept to the shared memory
/// the shared memory is released by the OS. So make sure to keep the handle until it's not needed anymore.
/// </summary>
/// <param name="buf">the buf to read the file from</param> 
/// <param name="bufSize">the size of the buffer</param> 
/// <returns></returns>
bool PublishFile(const char* buf, size_t bufSize, LPCSTR loc)
{
	// Please end my suffering
	const size_t total_size = sizeof(SHARED_FILE) + bufSize - 1;

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
		total_size,              // Maximum object size (low-order DWORD)
		loc);				     // Name of mapping object

	if (hMapFile == NULL) {
		PRINT_LAST_ERROR("Could not create file mapping object");
		return false;
	}

	LPVOID pBuf = MapViewOfFile(
		hMapFile,               // Handle to map object
		FILE_MAP_ALL_ACCESS,    // Read/write permission
		0,
		0,
		total_size);

	if (pBuf == NULL) {
		PRINT_LAST_ERROR("Could not map view of file");
		CloseHandle(hMapFile);
		return false;
	}

	LPSHARED_FILE sharedFile = (LPSHARED_FILE)pBuf;
	sharedFile->bufSize = bufSize;
	memcpy(sharedFile->fileBuf, buf, bufSize);

	UnmapViewOfFile(pBuf);

	return true;
}

bool GetFile(HANDLE* hMapFileOut, LPCSTR loc, char** bufOut, DWORD* bufLenOut) {
	HANDLE hMapFile = OpenFileMapping(
		FILE_MAP_READ,          // Read access
		FALSE,                  // Do not inherit the name
		loc);				    // Name of mapping object


	if (hMapFile == NULL) {
		PRINT_LAST_ERROR("Could not open file mapping object");
		return false;
	}

	LPVOID pBuf = MapViewOfFile(
		hMapFile,               // Handle to map object
		FILE_MAP_READ,          // Read permission
		0,
		0,
		0);

	if (pBuf == NULL) {
		PRINT_LAST_ERROR("Could not map view of file");
		CloseHandle(hMapFile);
		return false;
	}

	LPSHARED_FILE sharedFile = (LPSHARED_FILE)pBuf;
	*bufLenOut = sharedFile->bufSize;

	char* buffer = (char*)malloc(sharedFile->bufSize);
	if (buffer == NULL) {
		printf("Failed to allocate memory for buffer\n");
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

