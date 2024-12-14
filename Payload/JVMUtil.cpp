#include "JVMUtil.h"

jsize convert_to_jsize(DWORD size)
{
	return static_cast<jsize>(size);
}

jbyte* convert_to_jbyte(const char* raw_data, jlong size)
{
	jbyte* buffer = new jbyte[size];
	for (jlong i = 0; i < size; ++i) 
	{
		buffer[i] = static_cast<jbyte>(raw_data[i]);
	}
	return reinterpret_cast<jbyte*>(buffer);
}

jbyteArray convert_char_to_jbyte_array(JNIEnv* env, const char* buffer, size_t size)
{
    if (!env || !buffer || size == 0) 
    {
        return nullptr;
    }

    jbyteArray byte_array = env->NewByteArray(size);
    if (!byte_array)
    {
        return nullptr; 
    }

    env->SetByteArrayRegion(byte_array, 0, size, reinterpret_cast<const jbyte*>(buffer));
    return byte_array;
}

bool jentry_is_directory(JNIEnv* env, jobject jar_entry)
{   
	jclass jar_entry_class = env->GetObjectClass(jar_entry);
	if (!jar_entry_class)
	{
		printf("Failed to get class of JarEntry\n");
		return false;
	}
	jmethodID is_directory_method = env->GetMethodID(jar_entry_class, "isDirectory", "()Z");
	if (!is_directory_method)
	{
		printf("Failed to get isDirectory method\n");
		return false;
	}
	return env->CallBooleanMethod(jar_entry, is_directory_method);
}

bool clear_exception(JNIEnv* env) {
    jthrowable exception = env->ExceptionOccurred();
    if (exception != NULL) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }

    return exception != NULL;
}

void print_exception_stack_trace(JNIEnv* env) 
{
    jthrowable exception = env->ExceptionOccurred();
    if (!exception)
    {
        return; 
    }

    env->ExceptionClear();

    jclass throwableClass = env->FindClass("java/lang/Throwable");
    if (!throwableClass)
    {
        printf("Failed to find Throwable class.\n");
        return;
    }

    jmethodID printStackTraceMethod = env->GetMethodID(throwableClass, "printStackTrace", "(Ljava/io/PrintStream;)V");
    if (!printStackTraceMethod) 
    {
        printf("Failed to find printStackTrace method.\n");
        return;
    }

    jclass byteArrayOutputStreamClass = env->FindClass("java/io/ByteArrayOutputStream");
    jmethodID byteArrayOutputStreamCtor = env->GetMethodID(byteArrayOutputStreamClass, "<init>", "()V");
    jmethodID toStringMethod = env->GetMethodID(byteArrayOutputStreamClass, "toString", "()Ljava/lang/String;");

    jobject byteArrayOutputStream = env->NewObject(byteArrayOutputStreamClass, byteArrayOutputStreamCtor);

    jclass printStreamClass = env->FindClass("java/io/PrintStream");
    jmethodID printStreamCtor = env->GetMethodID(printStreamClass, "<init>", "(Ljava/io/OutputStream;)V");
    jobject printStream = env->NewObject(printStreamClass, printStreamCtor, byteArrayOutputStream);

    env->CallVoidMethod(exception, printStackTraceMethod, printStream);

    jstring stackTrace = (jstring)env->CallObjectMethod(byteArrayOutputStream, toStringMethod);
    const char* stackTraceChars = env->GetStringUTFChars(stackTrace, nullptr);

    printf("Java Exception Stack Trace:\n%s\n", stackTraceChars);

    env->ReleaseStringUTFChars(stackTrace, stackTraceChars);
    env->DeleteLocalRef(stackTrace);
    env->DeleteLocalRef(printStream);
    env->DeleteLocalRef(byteArrayOutputStream);
    env->DeleteLocalRef(exception);
}