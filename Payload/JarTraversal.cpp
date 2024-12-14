#include "JarTraversal.h"
#include <string>
#include <cstring>
#include <stdexcept>
#include <memory>
#include <vector>

#define MAGIC_NUMBER 0xCAFEBABE

bool jar_traversal_init(JarTraversalContext* context, JNIEnv* env, const char* jarFilePath)
{
	if (!context || !jarFilePath || !context->env) return false;

	try 
	{
		memset(context, 0, sizeof(JarTraversalContext));
		context->env = env;

		jclass fileInputStreamClass = env->FindClass("java/io/FileInputStream");
		jclass jarInputStreamClass = env->FindClass("java/util/jar/JarInputStream");

		CLEAR_EXCEPTION(env);
		if (!fileInputStreamClass || !jarInputStreamClass) 
		{
			throw std::runtime_error("Failed to load required Java classes.");
		}

		jmethodID fileInputStreamConstructor = env->GetMethodID(fileInputStreamClass, "<init>", "(Ljava/lang/String;)V");
		jmethodID jarInputStreamConstructor = env->GetMethodID(jarInputStreamClass, "<init>", "(Ljava/io/InputStream;)V");

		if (!fileInputStreamConstructor || !jarInputStreamConstructor) 
		{
			throw std::runtime_error("Failed to find required constructors.");
		}

		jstring jJarFilePath = env->NewStringUTF(jarFilePath);
		if (!jJarFilePath)
		{
			throw std::runtime_error("Failed to create Java string for JAR file path.");
		}

		jobject fileInputStream = env->NewObject(fileInputStreamClass, fileInputStreamConstructor, jJarFilePath);
		if (!fileInputStream)
		{
			throw std::runtime_error("Failed to create FileInputStream object.");
		}

		jobject jar_input_stream = env->NewObject(jarInputStreamClass, jarInputStreamConstructor, fileInputStream);
		if (!jar_input_stream)
		{
			throw std::runtime_error("Failed to create JarInputStream object.");
		}

		context->jar_input_stream = env->NewGlobalRef(jar_input_stream);
		context->has_more_entries = JNI_TRUE;

		env->DeleteLocalRef(fileInputStream);
		env->DeleteLocalRef(jJarFilePath);

		return true;

	}
	catch (const std::exception& ex)
	{
		printf("Error during jar_traversal_init: %s\n", ex.what());
		return false;
	}
}

bool jar_traversal_has_next(JarTraversalContext* context)
{
	if (!context || !context->jar_input_stream || !context->has_more_entries) return false;

	JNIEnv* env = context->env;

	try 
	{
		jclass jarInputStreamClass = env->GetObjectClass(context->jar_input_stream);
		jmethodID getNextEntryMethod = env->GetMethodID(jarInputStreamClass, "getNextEntry", "()Ljava/util/zip/ZipEntry;");

		jobject nextEntry = env->CallObjectMethod(context->jar_input_stream, getNextEntryMethod);
		context->current_entry = nextEntry ? env->NewGlobalRef(nextEntry) : nullptr;
		context->has_more_entries = (nextEntry != nullptr);

		return context->has_more_entries;

	}
	catch (...)
	{
		printf("Error in jar_traversal_has_next.\n");
		return false;
	}
}

JarEntryDetails* jar_traversal_next_entry(JarTraversalContext* context)
{
    if (!context || !context->current_entry)
    {
        return nullptr;
    }

    JNIEnv* env = context->env;

    try
    {
        JarEntryDetails* details = (JarEntryDetails*)malloc(sizeof(JarEntryDetails));
		if (!details)
		{
			printf("Failed to allocate memory for entry details.\n");
			return nullptr;
		}
        memset(details, 0, sizeof(JarEntryDetails));

        // Get entry name
        jclass zip_entry_class = env->GetObjectClass(context->current_entry);
        jmethodID get_name_method = env->GetMethodID(zip_entry_class, "getName", "()Ljava/lang/String;");
        if (!get_name_method)
        {
            printf("Failed to find getName method.\n");
            free(details);
            return nullptr;
        }

        jstring entry_name = (jstring)env->CallObjectMethod(context->current_entry, get_name_method);
        const char* name_chars = env->GetStringUTFChars(entry_name, nullptr);
        details->name = strdup(name_chars);
        env->ReleaseStringUTFChars(entry_name, name_chars);
        env->DeleteLocalRef(entry_name);


        // Read entry data
        std::vector<char> byte_data = std::vector<char>();
        jclass jar_input_stream_class = env->GetObjectClass(context->jar_input_stream);
        jmethodID read_method = env->GetMethodID(jar_input_stream_class, "read", "([B)I");
        if (!read_method)
        {
            printf("Failed to find read method.\n");
            return nullptr;
        }

		int buf_size = 8192;

        jbyteArray buffer = env->NewByteArray(buf_size);
        if (!buffer)
        {
            printf("Failed to allocate buffer.\n");
            return nullptr;
        }

		bool magic_number_checked = false;
		jint bytes_read;
        while ((bytes_read = env->CallIntMethod(context->jar_input_stream, read_method, buffer)) > 0)
        {
			// TODO: maybe insert a magic number check for class files here
            jbyte* buffer_data = env->GetByteArrayElements(buffer, nullptr);
			byte_data.insert(byte_data.end(), buffer_data, buffer_data + bytes_read);
            env->ReleaseByteArrayElements(buffer, buffer_data, JNI_ABORT);
        }

        env->DeleteLocalRef(buffer);

        // Populate details
        details->byte_data = (char*)malloc(byte_data.size());
		if (!details->byte_data)
		{
			printf("Failed to allocate memory for byte data.\n");
			return details;
		}

        memcpy(details->byte_data, byte_data.data(), byte_data.size());
        details->size = byte_data.size();

        return details;
    }
    catch (...)
    {
        printf("Error in jar_traversal_next_entry.\n");
        return nullptr;
    }
}

void jar_entry_details_free(JarTraversalContext* context, JarEntryDetails* details) 
{
	if (!details) return;

	if (details->name) free(details->name);
	if (details->byte_data) free(details->byte_data);
	free(details);
}

void jar_traversal_close(JarTraversalContext* context) 
{
	if (!context) return;

	JNIEnv* env = context->env;

	if (context->jar_input_stream)
	{
		env->DeleteGlobalRef(context->jar_input_stream);
		context->jar_input_stream = nullptr;
	}

	if (context->current_entry)
	{
		env->DeleteGlobalRef(context->current_entry);
		context->current_entry = nullptr;
	}
}
