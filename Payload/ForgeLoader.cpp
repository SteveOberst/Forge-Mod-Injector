#include "ForgeLoader.h"

#include <cstring>
#include <string>

#include "Defs.h"
#include "JVMUtil.h"
#include "JarTraversal.h"
#include "StringUtil.h"

static const char* const MAIN_THREAD = "Client thread";

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

jobject retrieve_launch_class_loader(jvmtiEnv* jvmti, JNIEnv* env)
{
	jint thread_count;
	jthread* threads;

	jvmti->GetAllThreads(&thread_count, &threads);
	for (int i = 0; i < thread_count; i++) {
		jthread thread = threads[i];
		jvmtiThreadInfo thread_info;
		jvmti->GetThreadInfo(thread, &thread_info);

		char* thread_name = thread_info.name;
		if (!strcmp(thread_name, MAIN_THREAD))
		{
			// we found the thread we're looking for - now all we need to do is get the context class loader
			jobject class_loader = get_context_class_loader(env, thread);
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

bool load_jar_into_classloader(jvmtiEnv* jvmti, JNIEnv* env, jobject class_loader, const char* jar_file_path)
{
	jclass class_loader_class;
	jfieldID resource_cache_field;
	jobject resource_cache;
	jfieldID cached_classes_field;
	jobject cached_classes;
	jclass map_class;
	jmethodID put_method;

	class_loader_class = env->GetObjectClass(class_loader);
	if (clear_exception(env))
	{
		printf("Error: Failed to get class of classLoader\n");
		return false;
	}

	resource_cache_field = env->GetFieldID(class_loader_class, "resourceCache", "Ljava/util/Map;");
	if (clear_exception(env))
	{
		printf("Error: resourceCache field not found\n");
		return false;
	}

	resource_cache = env->GetObjectField(class_loader, resource_cache_field);
	if (clear_exception(env))
	{
		printf("Error: resourceCache map is null\n");
		return false;
	}

	cached_classes_field = env->GetFieldID(class_loader_class, "cachedClasses", "Ljava/util/Map;");
	if (clear_exception(env))
	{
		printf("Error: cachedClasses field not found\n");
		return false;
	}

	cached_classes = env->GetObjectField(class_loader, cached_classes_field);
	if (clear_exception(env))
	{
		printf("Error: cachedClasses map is null\n");
		return false;
	}

	map_class = env->FindClass("java/util/Map");
	if (clear_exception(env))
	{
		printf("Error: java.util.Map class not found\n");
		return false;
	}

	put_method = env->GetMethodID(map_class, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
	if (clear_exception(env))
	{
		printf("Error: put method not found in Map\n");
		return false;
	}

	JarTraversalContext context;

	if (!jar_traversal_init(&context, env, jar_file_path))
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

		std::string converted_class_name = strings_convert_to_class_name(entry->name);
		jstring entry_name = env->NewStringUTF(converted_class_name.c_str());
		if (entry_name == NULL || clear_exception(env))
		{
			printf("Error: Failed to create Java String for entry name\n");
			jar_entry_details_free(&context, entry);
			continue;
		}

		// If the entry is a class file, define it and add it to the class loader
		if (strings_ends_with(entry->name, ".class"))
		{
			jclass defined_class = define_class(env, class_loader, strings_remove_class_suffix(entry->name).c_str(), entry->byte_data, entry->size);
			if (!defined_class) {
				printf("Error: Failed to define class: %s\n", entry->name);
				continue;
			}

			env->CallObjectMethod(cached_classes, put_method, entry_name, defined_class);
			if (env->ExceptionCheck())
			{
				env->ExceptionDescribe();
				env->ExceptionClear();
				printf("Error: Exception occurred while adding entry to cachedClasses\n");

			}

			continue;
		}

		// Just proceed normally for resources (non-class files)
		env->CallObjectMethod(resource_cache, put_method, entry_name, convert_char_to_jbyte_array(env, entry->byte_data, entry->size));

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

jclass define_class(JNIEnv* env, jobject class_loader, const char* class_name, char* class_bytes, size_t class_size)
{
	jclass classLoaderClass = env->GetObjectClass(class_loader);
	if (classLoaderClass == nullptr) {
		printf("Failed to get class of class loader\n");
		return false;
	}

	jclass definedClass = env->DefineClass(
		class_name,
		class_loader,
		convert_to_jbyte(class_bytes, class_size),
		convert_to_jsize(class_size)
	);

	if (definedClass == nullptr)
	{
		printf("Failed to define class\n");
		if (env->ExceptionOccurred())
		{
			print_exception_stack_trace(env);
		}
		else
		{
			printf("No exception was thrown, but DefineClass returned nullptr.");
		}

		return nullptr;
	}

	return definedClass;
}


jclass get_class(JNIEnv* env, jobject class_loader, const char* main_class)
{
	jclass class_loader_class = env->GetObjectClass(class_loader);
	if (!class_loader_class)
	{
		PRINTN("Failed to get class of class loader");
		return NULL;
	}

	jmethodID find_class_method = env->GetMethodID(class_loader_class, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");
	if (!find_class_method)
	{
		PRINTN("Failed to get method ID for findClass");
		return NULL;
	}

	jstring class_name = env->NewStringUTF(main_class);
	if (!class_name)
	{
		PRINTN("Failed to create Java string for class name");
		return NULL;
	}

	return (jclass)env->CallObjectMethod(class_loader, find_class_method, class_name);
}

bool call_main(JNIEnv* env, jobject launch_class_loader, const char* main_class_path)
{

	jclass main_class = get_class(env, launch_class_loader, main_class_path);
	if (!main_class)
	{
		if (env->ExceptionCheck())
		{
			print_exception_stack_trace(env);
			env->ExceptionDescribe();
			env->ExceptionClear();
			printf("An exception occurred while fetching main class.\n");
		}

		PRINTN("Failed to get main class");
		return false;
	}

	jmethodID main_method = env->GetStaticMethodID(main_class, "initialize", "()V");
	if (!main_method)
	{
		PRINTN("Failed to get main method");
		return false;
	}

	env->CallStaticVoidMethod(main_class, main_method);
	return true;
}
