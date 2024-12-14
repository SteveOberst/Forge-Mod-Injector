#include "Payload.h"

#include <random>

#include "string.h"
#include "jvmti.h"
#include "Shared.h"
#include "JarTraversal.h"
#include "StringUtil.h"
#include "JVMUtil.h"

#include "Defs.h"

#define TMP_FILE(file) (getTempFilePath(file))

const char* getTempFilePath(const char* fileName) {
	static char path[MAX_PATH];
	DWORD tempPathLen = GetTempPathA(MAX_PATH, path);
	if (tempPathLen == 0 || tempPathLen > MAX_PATH) {
		return NULL;
	}
	snprintf(path + tempPathLen, MAX_PATH - tempPathLen, "%s", fileName);
	return path;
}

char* filepath_to_java_url(const char* filepath) {
	if (filepath == NULL) {
		return NULL;
	}

	size_t len = strlen(filepath);

	size_t url_len = len + 7 + 1;
	char* java_url = (char*)malloc(url_len);

	if (java_url == NULL) {
		return NULL;
	}

	strcpy(java_url, "file://");

	for (size_t i = 0, j = 7; i < len; i++, j++) {
		if (filepath[i] == '\\') {
			java_url[j] = '/';  // Replace backslashes with forward slashes
		}
		else {
			java_url[j] = filepath[i];
		}
	}

	// Null-terminate the string
	java_url[url_len - 1] = '\0';

	return java_url;
}

static JNIEnv* g_env;
static jvmtiEnv* g_jvmti;
static JavaVM* g_jvm;

const char* const MAIN_THREAD = "Client thread";

void AllocateDebugConsole()
{
	AllocConsole();
	FILE* file;
	freopen_s(&file, "CONOUT$", "w", stdout);
	freopen_s(&file, "CONOUT$", "w", stderr);
}

bool clear_exception(JNIEnv* env) {
	jthrowable exception = env->ExceptionOccurred();
	if (exception != NULL) {
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	return exception != NULL;
}

jobject get_context_class_loader(JNIEnv* env, jthread thread) {
	jclass threadClass = env->GetObjectClass(thread);
	if (threadClass == NULL) {
		printf("Failed to get class of thread object.\n");
		return NULL;
	}

	jmethodID getContextClassLoaderMethod = env->GetMethodID(threadClass, "getContextClassLoader", "()Ljava/lang/ClassLoader;");
	if (getContextClassLoaderMethod == NULL) {
		printf("Failed to get method ID for getContextClassLoader.\n");
		return NULL;
	}

	jobject classLoader = env->CallObjectMethod(thread, getContextClassLoaderMethod);
	if (env->ExceptionCheck()) {
		env->ExceptionClear();
		printf("An exception occurred while calling getContextClassLoader.\n");
		return NULL;
	}

	return classLoader;
}

jobject retrieve_launch_class_loader()
{
	jint thread_count;
	jthread* threads;

	g_jvmti->GetAllThreads(&thread_count, &threads);
	for (int i = 0; i < thread_count; i++) {
		jthread thread = threads[i];
		jvmtiThreadInfo thread_info;
		g_jvmti->GetThreadInfo(thread, &thread_info);

		char* thread_name = thread_info.name;
		if (!strcmp(thread_name, MAIN_THREAD))
		{
			// we found the thread we're looking for - now all we need to do is get the context class loader
			jobject class_loader = get_context_class_loader(g_env, thread);
			if (!class_loader)
			{
				PRINTN("Failed to get context class loader from the client thread.");
			}

			return class_loader;
		}
	}

	// If we reach this point, we didn't find the main thread
	PRINTN("Failed to find the main thread.");
	return NULL;
}

jint initialize_vm_pointers() {
	jsize count;
	if (JNI_GetCreatedJavaVMs(&g_jvm, 1, &count) != JNI_OK || count == 0)
	{
		printf("Failed to get JVM\n");
		return JNI_FALSE;
	}

	jint res = g_jvm->GetEnv((void**)&g_env, JNI_VERSION_1_6);
	if (res == JNI_EDETACHED) 
	{
		printf("Attaching current thread to JVM\n");
		res = g_jvm->AttachCurrentThread((void**)&g_env, NULL);
		if (res != JNI_OK) 
		{
			printf("Failed to attach current thread to JVM\n");
			return JNI_FALSE;
		}
	}
	// Get the JVMTI environment
	if ((g_jvm)->GetEnv((void**)&g_jvmti, JVMTI_VERSION_1_0) != JNI_OK) 
	{
		printf("Failed to get JVMTI environment\n");
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

jclass define_class(jobject class_loader, const char* class_name, char* class_bytes, size_t class_size)
{
	jclass classLoaderClass = g_env->GetObjectClass(class_loader);
	if (classLoaderClass == nullptr) {
		printf("Failed to get class of class loader\n");
		return false;
	}

	jclass definedClass = g_env->DefineClass(
		class_name,
		class_loader,
		convert_to_jbyte(class_bytes, class_size),
		convert_to_jsize(class_size)
	);

	if (definedClass == nullptr)
	{
		printf("Failed to define class\n");
		if (g_env->ExceptionOccurred()) 
		{
			print_exception_stack_trace(g_env);
		}
		else 
		{
			printf("No exception was thrown, but DefineClass returned nullptr.");
		}

		return nullptr;
	}

	return definedClass;
}

bool load_jar_into_classloader(jvmtiEnv* jvmti, JNIEnv* env, jobject class_loader, const char* jar_file_path)
{
	jclass class_loader_class;
	jfieldID resource_cache_field;
	jobject resource_cache;
	jclass map_class;
	jmethodID put_method;

	class_loader_class = env->GetObjectClass(class_loader);
	if (clear_exception(g_env)) 
	{
		printf("Error: Failed to get class of classLoader\n");
		return false;
	}

	resource_cache_field = env->GetFieldID(class_loader_class, "resourceCache", "Ljava/util/Map;");
	if (clear_exception(g_env))
	{
		printf("Error: resourceCache field not found\n");
		return false;
	}

	resource_cache = env->GetObjectField(class_loader, resource_cache_field);
	if (clear_exception(g_env))
	{
		printf("Error: resourceCache map is null\n");
		return false;
	}

	map_class = env->FindClass("java/util/Map");
	if (clear_exception(g_env))
	{
		printf("Error: java.util.Map class not found\n");
		return false;
	}

	put_method = env->GetMethodID(map_class, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
	if (clear_exception(g_env)) 
	{
		printf("Error: put method not found in Map\n");
		return false;
	}

	JarTraversalContext context;

	if (!jar_traversal_init(&context, g_env, jar_file_path))
	{
		printf("Error: Failed to initialize JarTraversal\n");
		return false;
	}

	while (jar_traversal_has_next(&context)) 
	{
		JarEntryDetails* entry = jar_traversal_next_entry(&context);
		if (!entry) continue;

		if (jentry_is_directory(env, context.current_entry))
		{
			jar_entry_details_free(&context, entry);
			continue;
		}

		if (strings_ends_with(entry->name, "/")) 
		{
			jar_entry_details_free(&context, entry);
			continue;
		}

		// If the entry is a class file, define it and add it to the class loader
		if (strings_ends_with(entry->name, ".class"))
		{
			jclass defined_class = define_class(class_loader, strings_remove_class_suffix(entry->name).c_str(), entry->byte_data, entry->size);
			if (!defined_class) {
				printf("Error: Failed to define class: %s\n", entry->name);
				continue;
			}

			printf("Skipping path entry: %s\n", entry->name);

			printf("Loaded class: %s\n", entry->name);
			continue;
		}

		// Just proceed normally for resources (non-class files)
		std::string converted_class_name = strings_convert_to_class_name(entry->name);
		jstring entry_name = env->NewStringUTF(converted_class_name.c_str());
		printf("Processing entry: %s\n", entry->name);
		if (entry_name == NULL || clear_exception(g_env))
		{
			printf("Error: Failed to create Java String for entry name\n");
			jar_entry_details_free(&context, entry);
			continue;
		}

		env->CallObjectMethod(resource_cache, put_method, entry_name, convert_char_to_jbyte_array(g_env, entry->byte_data, entry->size));
		printf("Added entry to resourceCache: %s\n", converted_class_name.c_str());

		if (env->ExceptionCheck()) {
			env->ExceptionDescribe();
			env->ExceptionClear();
			printf("Error: Exception occurred while adding entry to resourceCache\n");
		}

		// Cleanup
		env->DeleteLocalRef(entry_name);
		jar_entry_details_free(&context, entry);
	}

	jar_traversal_close(&context);

	env->DeleteLocalRef(class_loader_class);
	env->DeleteLocalRef(map_class);

	printf("JAR file contents successfully registered in resourceCache\n");
	return true;
}


jclass get_class(jobject class_loader, const char* main_class)
{
	jclass class_loader_class = g_env->GetObjectClass(class_loader);
	if (!class_loader_class)
	{
		PRINTN("Failed to get class of class loader");
		return NULL;
	}

	jmethodID find_class_method = g_env->GetMethodID(class_loader_class, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");
	if (!find_class_method)
	{
		PRINTN("Failed to get method ID for findClass");
		return NULL;
	}

	jstring class_name = g_env->NewStringUTF(main_class);
	if (!class_name)
	{
		PRINTN("Failed to create Java string for class name");
		return NULL;
	}

	return (jclass)g_env->CallObjectMethod(class_loader, find_class_method, class_name);
}

BOOL DLLEXPORT WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason != DLL_PROCESS_ATTACH)
	{
		return FALSE;
	}

#ifdef _DEBUG
	AllocateDebugConsole();
#endif
	printf("Payload DLL injected\n");

	HANDLE hParamMapFile = NULL;
	DWORD dwParamBufSize = NULL;
	char* main_class_path = nullptr;
	bool get_param_res = GetFile(&hParamMapFile, MAIN_CLASS_LOC, &main_class_path, &dwParamBufSize);
	if (!get_param_res)
	{
		PRINTN("Failed to fetch main class from shared memory");
		goto error;
	}

	printf("Main class path: %s\n", main_class_path);

	jint res = initialize_vm_pointers();
	if (!res)
	{
		PRINTN("Failed to initialize VM pointers. Exiting...");
		return FALSE;
	}

	PRINTN("Successfully initialized VM pointers. Running payload...");

	HANDLE hMapFile = NULL;
	DWORD dwBufSize = NULL;
	char* mod_buf = nullptr;
	// Here we could technically proceed with injecting the in-memory JAR file, however, I will not
	// showcase this here and instead just write the JAR file to disk
	bool get_file_res = GetFile(
		&hMapFile,
		MOD_FILE_LOC,
		&mod_buf,
		&dwBufSize
	);
	if (!get_file_res)
	{
		PRINTN("Failed to get file from shared memory");
		free(mod_buf);
		goto error;
	}

	const char* output_file = "C:\\Users\\baumg\\Desktop\\test.jar"; //TMP_FILE("Payload.jar");
	if (!output_file)
	{
		PRINTN("Failed to resolve temp file path");
		CloseHandle(hMapFile);
		free(mod_buf);
		goto error;
	}

	FILE* file = fopen(output_file, "wb");
	if (!file)
	{
		PRINTN("Failed to create JAR file on disk");
		CloseHandle(hMapFile);
		free(mod_buf);
		goto error;
	}

	size_t written_bytes = fwrite(mod_buf, 1, dwBufSize, file);
	if (!written_bytes)
	{
		PRINTN("Failed to write JAR file to disk");
		CloseHandle(hMapFile);
		free(mod_buf);
		goto error;
	}

	fclose(file);
	jobject launch_class_loader = retrieve_launch_class_loader();
	if (!launch_class_loader)
	{
		PRINTN("Failed to hijack class loader");
		CloseHandle(hMapFile);
		free(mod_buf);
		goto error;
	}

	printf("Loading jar into class loader...");

	bool load_jar_res = load_jar_into_classloader(
		g_jvmti,
		g_env,
		launch_class_loader,
		output_file
	);
	if (!load_jar_res)
	{
		PRINTN("Failed to load JAR file into class loader");
		CloseHandle(hMapFile);
		free(mod_buf);
		goto error;
	}

	printf("JAR file loaded into class loader. Fetching main class...\n");

	jclass main_class = get_class(launch_class_loader, main_class_path);
	if (!main_class)
	{
		if (g_env->ExceptionCheck())
		{
			print_exception_stack_trace(g_env);
			g_env->ExceptionDescribe();
			g_env->ExceptionClear();
			printf("An exception occurred while fetching main class.\n");
		}

		PRINTN("Failed to get main class");
		CloseHandle(hMapFile);
		free(mod_buf);
		goto error;
	}

	jmethodID main_method = g_env->GetStaticMethodID(main_class, "initialize", "()V");
	if (!main_method)
	{
		PRINTN("Failed to get main method");
		CloseHandle(hMapFile);
		free(mod_buf);
		goto error;
	}

	g_env->CallStaticVoidMethod(main_class, main_method);

	FreeLibrary(GetModuleHandleA(NULL));
	CloseHandle(hMapFile);
	free(mod_buf);
	return TRUE;

error:
	FreeLibrary(GetModuleHandleA(NULL));
	return FALSE;
}