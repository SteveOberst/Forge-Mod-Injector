#include "Payload.h"

#include <random>

#include "ForgeLoader.h"

#include "Shared.h"
#include "JarTraversal.h"
#include "JVMUtil.h"
#include "Util.h"

#include "Defs.h"

static JNIEnv* g_env;
static jvmtiEnv* g_jvmti;
static JavaVM* g_jvm;

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

BOOL DLLEXPORT WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason != DLL_PROCESS_ATTACH)
	{
		return FALSE;
	}

#ifdef _DEBUG
	AllocateDebugConsole();
#endif
	HANDLE hParamMapFile = NULL;
	DWORD dwParamBufSize = NULL;
	char* main_class_path = nullptr;
	bool get_param_res = GetFile(&hParamMapFile, MAIN_CLASS_LOC, &main_class_path, &dwParamBufSize);
	if (!get_param_res)
	{
		PRINTN("Failed to fetch main class from shared memory");
		goto error;
	}

	jint res = initialize_vm_pointers();
	if (!res)
	{
		PRINTN("Failed to initialize VM pointers. Exiting...");
		return FALSE;
	}

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
	jobject launch_class_loader = retrieve_launch_class_loader(g_jvmti, g_env);
	if (!launch_class_loader)
	{
		PRINTN("Failed to hijack class loader");
		CloseHandle(hMapFile);
		free(mod_buf);
		goto error;
	}

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

	if (!call_main(g_env, launch_class_loader, main_class_path))
	{
		PRINTN("Failed to call main method");
		CloseHandle(hMapFile);
		free(mod_buf);
		goto error;
	}

	FreeLibrary(GetModuleHandleA(NULL));
	CloseHandle(hMapFile);
	free(mod_buf);
	return TRUE;

error:
	FreeLibrary(GetModuleHandleA(NULL));
	return FALSE;
}