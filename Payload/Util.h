#pragma once

#define TMP_FILE(file) (getTempFilePath(file))

const char* getTempFilePath(const char* fileName);

void AllocateDebugConsole();