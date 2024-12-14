#pragma once

#include <Windows.h>
#include <jni.h>

jsize convert_to_jsize(DWORD size);

jbyte* convert_to_jbyte(const char* raw_data, jlong size);

jbyteArray convert_char_to_jbyte_array(JNIEnv* env, const char* buffer, size_t size);

bool jentry_is_directory(JNIEnv* env, jobject jar_entry);

void print_exception_stack_trace(JNIEnv* env);