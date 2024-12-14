#pragma once

#include "jvmti.h"
#include "jni.h"
#include "Defs.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" 
{
#endif

	typedef struct 
	{
		JNIEnv* env;
		jobject jar_input_stream;
		jboolean has_more_entries;
		jobject current_entry;
	} JarTraversalContext;

	typedef struct 
	{
		char* name;
		char* byte_data;
		size_t size;
	} JarEntryDetails;

	DLLEXPORT bool jar_traversal_init(JarTraversalContext* context, JNIEnv* env, const char* jarFilePath);

	DLLEXPORT bool jar_traversal_has_next(JarTraversalContext* context);

	DLLEXPORT JarEntryDetails* jar_traversal_next_entry(JarTraversalContext* context);

	DLLEXPORT void jar_entry_details_free(JarTraversalContext* context, JarEntryDetails* details);

	DLLEXPORT void jar_traversal_close(JarTraversalContext* context);

#ifdef __cplusplus
}
#endif
