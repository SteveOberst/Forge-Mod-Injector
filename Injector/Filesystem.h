#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

bool check_file_exists(const char* file_path);

char* read_file(const char* file_path, size_t* buf_len);