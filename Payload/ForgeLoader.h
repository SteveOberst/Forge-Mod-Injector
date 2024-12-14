#pragma once

#include "jni.h"
#include "jvmti.h"

jobject get_context_class_loader(JNIEnv* env, jthread thread);

jobject retrieve_launch_class_loader(jvmtiEnv* jvmti, JNIEnv* env);

bool load_jar_into_classloader(jvmtiEnv* jvmti, JNIEnv* env, jobject class_loader, const char* jar_file_path);

jclass define_class(JNIEnv* env, jobject class_loader, const char* class_name, char* class_bytes, size_t class_size);

jclass get_class(JNIEnv* env, jobject class_loader, const char* main_class);

bool call_main(JNIEnv* env, jobject launch_class_loader, const char* main_class);