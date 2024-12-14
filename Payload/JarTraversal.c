#include "JarTraversal.h"
#include <string.h>
#include <stdlib.h>

static void ClearException(JNIEnv* env) {
	jthrowable exception = (*env)->ExceptionOccurred(env);
	if (exception) {
		(*env)->ExceptionDescribe(env);
		(*env)->ExceptionClear(env);
	}
}

bool jar_traversal_init(JarTraversalContext* context, const char* jarFilePath) {
    if (!context || !jarFilePath) return false;

    memset(context, 0, sizeof(JarTraversalContext));

    JavaVM* jvm;
    jint res = JNI_GetCreatedJavaVMs(&jvm, 1, NULL);
    if (res != JNI_OK || !jvm) return false;

    (*jvm)->AttachCurrentThread(jvm, (void**)&context->env, NULL);

    jclass fileInputStreamClass = (*context->env)->FindClass(context->env, "java/io/FileInputStream");
    jclass jarInputStreamClass = (*context->env)->FindClass(context->env, "java/util/jar/JarInputStream");

    if (!fileInputStreamClass || !jarInputStreamClass) return false;

    jmethodID fileInputStreamConstructor = (*context->env)->GetMethodID(context->env, fileInputStreamClass, "<init>", "(Ljava/lang/String;)V");
    jmethodID jarInputStreamConstructor = (*context->env)->GetMethodID(context->env, jarInputStreamClass, "<init>", "(Ljava/io/InputStream;)V");

    jstring jJarFilePath = (*context->env)->NewStringUTF(context->env, jarFilePath);
    jobject fileInputStream = (*context->env)->NewObject(context->env, fileInputStreamClass, fileInputStreamConstructor, jJarFilePath);

    context->jar_input_stream = (*context->env)->NewObject(context->env, jarInputStreamClass, jarInputStreamConstructor, fileInputStream);

    if (!context->jar_input_stream) return false;

    context->has_more_entries = JNI_TRUE;
    return true;
}

bool jar_traversal_has_next(JarTraversalContext* context) {
    if (!context || !context->jar_input_stream) return false;

    jclass jarInputStreamClass = (*context->env)->GetObjectClass(context->env, context->jar_input_stream);
    jmethodID getNextEntryMethod = (*context->env)->GetMethodID(context->env, jarInputStreamClass, "getNextEntry", "()Ljava/util/jar/JarEntry;");

    context->current_entry = (*context->env)->CallObjectMethod(context->env, context->jar_input_stream, getNextEntryMethod);
    context->has_more_entries = (context->current_entry != NULL);

    return context->has_more_entries;
}

char* jar_traversal_next_entry_name(JarTraversalContext* context) {
    if (!context || !context->current_entry) return NULL;

    jclass jarEntryClass = (*context->env)->GetObjectClass(context->env, context->current_entry);
    jmethodID getNameMethod = (*context->env)->GetMethodID(context->env, jarEntryClass, "getName", "()Ljava/lang/String;");

    jstring entryName = (jstring)(*context->env)->CallObjectMethod(context->env, context->current_entry, getNameMethod);
    const char* entryNameUTF = (*context->env)->GetStringUTFChars(context->env, entryName, NULL);

    char* result = strdup(entryNameUTF);
    (*context->env)->ReleaseStringUTFChars(context->env, entryName, entryNameUTF);

    return result;
}

unsigned char* jar_traversal_read_entry_bytes(JarTraversalContext* context, size_t* length) {
    if (!context || !context->jar_input_stream || !length) return NULL;

    jclass jarInputStreamClass = (*context->env)->GetObjectClass(context->env, context->jar_input_stream);
    jmethodID readMethod = (*context->env)->GetMethodID(context->env, jarInputStreamClass, "read", "([B)I");

    const size_t bufferSize = 8192;
    unsigned char* buffer = malloc(bufferSize);
    if (!buffer) return NULL;

    jbyteArray javaBuffer = (*context->env)->NewByteArray(context->env, bufferSize);
    *length = 0;

    int bytesRead;
    while ((bytesRead = (*context->env)->CallIntMethod(context->env, context->jar_input_stream, readMethod, javaBuffer)) > 0) {
        buffer = realloc(buffer, *length + bytesRead);
        (*context->env)->GetByteArrayRegion(context->env, javaBuffer, 0, bytesRead, (jbyte*)(buffer + *length));
        *length += bytesRead;
    }

    return buffer;
}

void jar_traversal_close(JarTraversalContext* context) {
    if (!context || !context->jar_input_stream) return;

    jclass jarInputStreamClass = (*context->env)->GetObjectClass(context->env, context->jar_input_stream);
    jmethodID closeMethod = (*context->env)->GetMethodID(context->env, jarInputStreamClass, "close", "()V");

    (*context->env)->CallVoidMethod(context->env, context->jar_input_stream, closeMethod);

    JavaVM* jvm;
    (*context->env)->GetJavaVM(context->env, &jvm);
    (*jvm)->DetachCurrentThread(jvm);
}
