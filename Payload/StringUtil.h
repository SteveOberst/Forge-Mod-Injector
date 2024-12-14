#pragma once

#include <string>

bool strings_ends_with(const char* str, const char* suffix);

std::string strings_convert_to_class_name(const char* class_path);

std::string strings_remove_class_suffix(const char* class_path);