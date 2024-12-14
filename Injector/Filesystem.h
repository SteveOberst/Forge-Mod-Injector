#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

	bool check_file_exists(const char* file_path);

	char* read_file(const char* file_path, size_t* buf_len);

	int is_absolute_path(const char* path);

	char* make_absolute_path(const char* path);

	void get_file_name(const char* file_path, char* file_name, size_t buffer_size);

#ifdef __cplusplus
}
#endif