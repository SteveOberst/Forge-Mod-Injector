#include "Payload.h"
#include "string.h"
#include "jvmti.h"

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

    g_jvmti->GetAllThreads(&thread_count, & threads);
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
	if (JNI_GetCreatedJavaVMs(&g_jvm, 1, &count) != JNI_OK || count == 0) {
		printf("Failed to get JVM\n");
		return JNI_FALSE;
	}

    jint res = g_jvm->GetEnv((void**)&g_env, JNI_VERSION_1_6);
    if (res == JNI_EDETACHED) {
		printf("Attaching current thread to JVM\n");
        res = g_jvm->AttachCurrentThread((void**)&g_env, NULL);
        if (res != JNI_OK) {
            printf("Failed to attach current thread to JVM\n");
            return JNI_FALSE;
        }
    }
    // Get the JVMTI environment
    if ((g_jvm)->GetEnv((void**)&g_jvmti, JVMTI_VERSION_1_0) != JNI_OK) {
        printf("Failed to get JVMTI environment\n");
        return JNI_FALSE;
    }

	return JNI_TRUE;
}

bool load_jar_into_classloader(jvmtiEnv* jvmti, JNIEnv* env, jobject classLoader, const char* jarFilePath) {
    jclass launch_class_loader_class;
    jmethodID add_url_method;
    jstring jar_path_string;

    launch_class_loader_class = env->FindClass("net/minecraft/launchwrapper/LaunchClassLoader");
    if (launch_class_loader_class == NULL) {
        printf("Error: LaunchClassLoader class not found\n");
        return false;
    }


}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason != DLL_PROCESS_ATTACH)
    {
        return FALSE;
    }

#ifdef _DEBUG
	AllocateDebugConsole();
#endif

	printf("Injected\n");
    jint res = initialize_vm_pointers();
    if (!res)
    {
        PRINTN("Failed to initialize VM pointers. Exiting...");
        return FALSE;
    }

    PRINTN("Successfully initialized VM pointers. Running payload...");
    
    jobject launch_class_loader = retrieve_launch_class_loader();
    if (!launch_class_loader)
    {
        PRINTN("Failed to hijack class loader");
        return FALSE;
    }

    load_jar_into_classloader(g_jvmti, g_env, launch_class_loader, "C:\\Users\\user\\Desktop\\test.jar");



	printf("unloading\n");
    FreeLibrary(GetModuleHandleA(NULL));
    return TRUE;
}
